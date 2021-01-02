#include "app.h"

#include <QFinalState>
#include <QSettings>

#include <QTimer>
#include <QtDebug>

#include <QGpgME/ChangeOwnerTrustJob>
#include <QGpgME/CryptoConfig>
#include <QGpgME/DecryptJob>
#include <QGpgME/ExportJob>
#include <QGpgME/ImportFromKeyserverJob>
#include <QGpgME/KeyGenerationJob>
#include <QGpgME/KeyListJob>
#include <QGpgME/ListAllKeysJob>
#include <QGpgME/SignJob>
#include <QGpgME/SignKeyJob>

#include <QGpgME/EncryptJob>
#include <QGpgME/Protocol>

#include <gpgme++/keygenerationresult.h>
#include <gpgme++/keylistresult.h>

App::App(QObject *parent)
    : QObject(parent)
{
    setupFSM();
    setupInitFSM();
    connect(&m_initFSM, &QStateMachine::finished, this, &App::registrationFinished, Qt::UniqueConnection);
    connect(this, &App::appStateChanged, this, [this]() noexcept { qDebug() << "appState:" << this->appState(); });
    connect(this, &App::initStateChanged, this, [this]() noexcept { qDebug() << "initState:" << this->initState(); });
}

void App::startRegistration(const QString &name, const QString &address)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_initAccountData = {};
    m_initAccountData.address = address;
    m_initAccountData.company = name;
    m_initFSM.start();
    emit registrationStarted();
}

void App::initCrypto()
{
    qDebug() << __PRETTY_FUNCTION__;
    auto *job = QGpgME::openpgp()->listAllKeysJob(false, false);
    connect(job, &QGpgME::ListAllKeysJob::result, this,
            [](const GpgME::KeyListResult &result, const std::vector< GpgME::Key > &pub = std::vector< GpgME::Key >(),
               const std::vector< GpgME::Key > &sec = std::vector< GpgME::Key >(), const QString &auditLogAsHtml = QString(),
               const GpgME::Error &auditLogError = GpgME::Error()) noexcept {
                for (const auto &key : pub) {
                    qInfo() << "key:" << key.keyID() << key.primaryFingerprint();
                }
            });
    job->start(false);
}

namespace
{
    QState *onEnter(QState *state, std::function< void() > action)
    {
        QObject::connect(state, &QAbstractState::entered, state->parent(), action);
        return state;
    }
}// namespace
void App::setupFSM()
{
    auto *init = addState("init");
    onEnter(init, [this]() noexcept {
        loadSettings();
        emit registeredChanged();
    });
    m_fsm.setInitialState(init);
    auto *idle = addState("idle");
    auto *registration = addState("registration");
    auto *exit = new QFinalState(&m_fsm);
    connect(exit, &QAbstractState::entered, this, [this]() noexcept { this->setProperty("appState", "exit"); });
    init->addTransition(idle);
    idle->addTransition(this, &App::registrationStarted, registration);
    registration->addTransition(this, &App::registrationFinished, idle);
}

QState *App::addState(const char *name)
{
    auto *state = new QState(&m_fsm);
    state->connect(state, &QState::entered, this, [this, name]() { this->setAppState(name); });
    return state;
}

void App::setupInitFSM()
{
    QState *init = addInitState("getting_uuid");
    m_initFSM.setInitialState(init);
    QState *generating = addInitState("generating");
    QState *registering = addInitState("registering");
    QState *success = addInitState("success");
    QState *error = addInitState("error");
    QFinalState *exit = new QFinalState(&m_initFSM);

    init->addTransition(this, &App::uuidCreated, generating);
    init->addTransition(this, &App::serverError, error);

    generating->addTransition(this, &App::keysCreated, registering);
    generating->addTransition(this, &App::cryptoError, error);

    registering->addTransition(this, &App::userRegistered, success);
    registering->addTransition(this, &App::serverError, error);

    error->addTransition(exit);
    success->addTransition(exit);

    onEnter(init, [this]() noexcept {
        setRegistering(true);
        // create UUID
        QTimer::singleShot(2000, this, &App::uuidCreated);
    });
    onEnter(generating, [this]() noexcept {
        // generate keys
        auto *job = QGpgME::openpgp()->keyGenerationJob();
        connect(job, &QGpgME::KeyGenerationJob::result, this,
                [this](const GpgME::KeyGenerationResult &result, const QByteArray &pubKeyData,
                       const QString &auditLogAsHtml = QString(),
                       const GpgME::Error &auditLogError = GpgME::Error()) noexcept {
                    qInfo() << "error:" << result.error().asString() << "pubKeyData" << pubKeyData
                            << "isPrimaryKeyGenerated=" << result.isPrimaryKeyGenerated()
                            << "fingerprint=" << result.fingerprint();
                    m_userData.fingerPrint = result.fingerprint();
                    m_userData.publicKey = pubKeyData;
                    auto *job = QGpgME::openpgp()->publicKeyExportJob(false);
                    connect(job, &QGpgME::ExportJob::result, this,
                            [this](const GpgME::Error &result, const QByteArray &keyData,
                                   const QString &auditLogAsHtml = QString(),
                                   const GpgME::Error &auditLogError = GpgME::Error()) noexcept {
                                qInfo() << "error=" << result.asString();
                                qInfo() << "pubKey:" << keyData;
                                m_initAccountData.publicKey = keyData;
                                m_userData.publicKey = keyData;
                            });
                    job->start(QStringList() << QString::fromUtf8(m_userData.fingerPrint));
                    emit keysCreated();
                });
        const char *parameters = "<GnupgKeyParms format=\"internal\">\n"
                                 "Key-Type: DSA\n"
                                 "Key-Length: 1024\n"
                                 "Subkey-Type: ELG-E\n"
                                 "Subkey-Length: 1024\n"
                                 "Name-Real: Joe Tester\n"
                                 "Name-Comment: (pp=abc)\n"
                                 "Name-Email: joe@foo.bar\n"
                                 "Passphrase: abc\n"
                                 "Expire-Date: 0\n"
                                 "</GnupgKeyParms>\n";
        job->start(parameters);
    });
    onEnter(registering, [this]() noexcept {
        // register keys
        QTimer::singleShot(2000, this, &App::userRegistered);
    });
    connect(&m_initFSM, &QStateMachine::finished, this, [this]() noexcept {
        setRegistering(false);
        emit this->registrationFinished();
    });
}

QState *App::addInitState(const char *name)
{
    auto *state = new QState(&m_initFSM);
    state->connect(state, &QState::entered, this, [this, name]() { this->setInitState(name); });
    return state;
}

void App::loadSettings()
{
    m_userData.uuid = QSettings().value("uuid", QUuid()).toUuid();
}

void App::storeSettings()
{
    QSettings settings;
    settings.setValue("uuid", m_initAccountData.uuid);
    settings.setValue("private_key", m_initAccountData.privateKey);
    settings.setValue("public_key", m_initAccountData.publicKey);
}

void App::setRegistering(bool registering)
{
    if (m_registering != registering) {
        m_registering = registering;
        emit registeringChanged();
    }
}
