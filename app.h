#pragma once
#include <QObject>
#include <QStateMachine>
#include <QUuid>
#include <functional>

class App : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int total READ total NOTIFY totalChanged)
    Q_PROPERTY(int numUnionConfirmed READ numUnionConfirmed NOTIFY numUnionConfirmedChanged)
    Q_PROPERTY(int numPeerConfirmed READ numPeerConfirmed NOTIFY numPeerConfirmedChanged)
    Q_PROPERTY(QString initState READ initState NOTIFY initStateChanged)
    Q_PROPERTY(bool registering READ registering NOTIFY registeringChanged)
    Q_PROPERTY(QString appState READ appState NOTIFY appStateChanged)
    Q_PROPERTY(bool registered READ registered NOTIFY registeredChanged)
    Q_PROPERTY(QString uuid READ uuidText NOTIFY uuidChanged)
    //    Q_PROPERTY(QString initState READ initState NOTIFY initStateChanged)
public:
    explicit App(QObject *parent = nullptr);

    int total() const
    {
        return m_total;
    }
    int numUnionConfirmed() const
    {
        return m_numUnionConfirmed;
    }
    int numPeerConfirmed() const
    {
        return m_numPeerConfirmed;
    }
    bool registering() const
    {
        return m_registering;
    }
    QString appState() const
    {
        return m_appState;
    }
    QString initState() const
    {
        return m_initState;
    }
    bool registered() const
    {
        return !m_userData.uuid.isNull() && !m_userData.privateKey.isEmpty() && !m_userData.publicKey.isEmpty();
    }

    Q_INVOKABLE void startRegistration(const QString &name, const QString &address);
    void initCrypto();
    Q_INVOKABLE void start()
    {
        initCrypto();
        m_fsm.start();
    }
    Q_INVOKABLE void requestSetupPage()
    {
        emit setupPageRequested();
    }

    QString uuidText() const
    {
        return m_userData.uuid.toString();
    }

private:
    void setupFSM();
    QState *addState(const char *name);

    void setupInitFSM();
    QState *addInitState(const char *name);
    void loadSettings();
    void storeSettings();

    void setRegistering(bool registering);
    void setAppState(const char *state)
    {
        m_appState = state;
        emit appStateChanged();
    }
    void setInitState(const char *state)
    {
        m_initState = state;
        emit initStateChanged();
    }

signals:
    void setupPageRequested();
    void totalChanged();
    void numUnionConfirmedChanged();
    void numPeerConfirmedChanged();
    void appStateChanged();
    void initStateChanged();
    void registeredChanged();
    void registeringChanged();
    void registrationStarted();
    void registrationSuccessful();
    void registrationFinished();
    void registrationFailed();
    void uuidCreated();
    void keysCreated();
    void userRegistered();

    void serverError();
    void cryptoError();

    void uuidChanged();

private:
    int m_total = 0;
    int m_numUnionConfirmed = 0;
    int m_numPeerConfirmed = 0;

    QStateMachine m_fsm;
    QStateMachine m_initFSM;

    bool m_registering = false;
    const char *m_appState = "";
    const char *m_initState = "";

    struct {
        QUuid uuid;
        QByteArray fingerPrint;
        QByteArray publicKey;
        QByteArray privateKey;
    } m_userData;

    struct {
        QString company;
        QString address;
        QByteArray publicKey;
        QByteArray privateKey;
        QUuid uuid;
    } m_initAccountData;

    Q_DISABLE_COPY(App)
};
