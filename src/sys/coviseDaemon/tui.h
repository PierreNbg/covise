#ifndef VRB_REMOTE_LAUNCHER_TUI_H
#define VRB_REMOTE_LAUNCHER_TUI_H

#include "metaTypes.h"
#include "tuiCommands.h"
#include "coviseDaemon.h"

#include <vrb/client/VrbCredentials.h>

#include <QObject>
#include <QMainWindow>
#include <QSocketNotifier>

#include <string>
#include <atomic>
#include <mutex>
#include <memory>

class CommandLineUi : public QObject
{
    Q_OBJECT
public:
    CommandLineUi(const vrb::VrbCredentials &credentials, bool autostart);
   
    private:
        CoviseDaemon m_launcher;
        std::atomic_bool m_launchDialog{false};
        std::mutex m_mutex;
        std::vector<std::string> m_args;
        int m_senderId;
        vrb::Program m_program;
        bool m_autostart = false;
        std::vector<std::unique_ptr<CommandInterface>> m_commands;
        QSocketNotifier m_cinNotifier;
        void handleCommand(const std::string &command);
        void createCommands();
};

#endif // !VRB_REMOTE_LAUNCHER_TUI_H