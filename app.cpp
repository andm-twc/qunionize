#include "app.h"

#include <QFinalState>
#include <QSettings>

#include <QTimer>
#include <QtDebug>

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
        QTimer::singleShot(2000, this, &App::keysCreated);
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