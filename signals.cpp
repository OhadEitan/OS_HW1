#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	////Ctrl+Z causes the shell to send SIGTSTP to the process running in the foreground
    // TODO: Add your implementation
    SmallShell& smash = SmallShell::getInstance();
    cout << "smash: got ctrl-Z" << endl;
    if(smash.current_fg_pid >= 0)
    {
        smash.jobs_list.getJobByPID(smash.current_fg_pid)->is_stopped = true;
        kill(smash.current_fg_pid, SIGSTOP);
        cout << "smash: process " << smash.current_fg_pid << " was stopped" << endl;
        smash.current_fg_pid = -1;
    }
}

void ctrlCHandler(int sig_num) {
    //// Ctrl+C causes the shell to send SIGINT to the process running in the foreground
  // TODO: Add your implementation
    SmallShell& smash = SmallShell::getInstance();
    cout << "smash: got ctrl-C" << endl;
    if(smash.current_fg_pid >= 0) {
        //smash.jobs_list.getJobByPID(smash.current_fg_pid)->is_stopped = true;
       //// maybe signal here is not right
        kill(smash.current_fg_pid, SIGKILL);
        cout << "smash: process " << smash.current_fg_pid << " was killed" << endl;
        smash.current_fg_pid = -1;

    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
    SmallShell& smash = SmallShell::getInstance();
    cout << "smash: got an alarm" << endl;
//    smash.
//    ((SmallShell*)running_smash)->handle_alarm();


}

