/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef VRB_REMOTE_LAUNCHER_MESSAGE_HANDLER_H
#define VRB_REMOTE_LAUNCHER_MESSAGE_HANDLER_H

#include "childProcess.h"
#include "metaTypes.h"

#include <vrb/ProgramType.h>
#include <vrb/client/VrbCredentials.h>
#include <vrb/client/VRBClient.h>

#include <QObject>
#include <QSocketNotifier>
#include <QString>

#include <atomic>
#include <functional>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

//The CoviseDaemon that listens to launch request submitted via VRB
class CoviseDaemon : public QObject
{
    Q_OBJECT
public:
    ~CoviseDaemon();
    void connect(const vrb::VrbCredentials &credentials = vrb::VrbCredentials{});
    void disconnect();
    void printClientInfo();
    void sendPermission(int clientID, bool permit);
    void spawnProgram(vrb::Program p, const std::vector<std::string> &args, const std::function<void(const QString& output)> outputCallback, const std::function<void(void)> &diedCallback);
public slots:
    void sendLaunchRequest(vrb::Program p, int lientID, const std::vector<std::string> &args = std::vector<std::string>{});
signals:
    void launchSignal(int senderID, QString senderDescription, vrb::Program programID, std::vector<std::string> startOptions); //namespace must be explicitly stated for qRegisterMetaType
    void connectedSignal();
    void disconnectedSignal();
    void updateClient(int clientID, QString clientInfo);
    void removeClient(int id);

private:
    typedef std::lock_guard<std::mutex> Guard;
    std::atomic<bool> m_terminate{false};
    std::atomic<bool> m_shouldBeConnected{false};
    std::unique_ptr<vrb::VrbCredentials> m_credentials;
    bool m_newCredentials = false;
    std::unique_ptr<vrb::VRBClient> m_client = nullptr;
    std::unique_ptr<std::thread> m_thread;
    std::mutex m_mutex;

    vrb::SessionID m_sessionID;
    std::set<vrb::RemoteClient> m_clientList;
    std::set<ChildProcess> m_children;

    void loop();
    bool handleVRB();
    bool removeOtherClient(covise::TokenBuffer &tb);
    void handleVrbLauncherMessage(covise::Message &msg);
    std::set<vrb::RemoteClient>::iterator findClient(int id);
    
};
QString getClientInfo(const vrb::RemoteClient &cl);


#endif