#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
//#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"


#include <sys/wait.h>
#include <fcntl.h>
#include <sched.h>
#include <thread>

using namespace std;
#define MAX_ARGS_IN_CMD 20
#define MAX_COMMAND_LENGTH 80
//JobsList::JobEntry * SmallShell::current_jobs;
//vector<JobsList::JobEntry *> JobsList::jobs;
//vector<JobsList::JobEntry *> JobsList::times;
//SmallShell small;


const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif




string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

Command::Command(const char* cmd_line)
{
    this->arguments = new char *[MAX_ARGS_IN_CMD];
    this->cmd_line=new char[MAX_COMMAND_LENGTH];
    this->cmd_line= strcpy(this->cmd_line,cmd_line);
    this->args_size = _parseCommandLine(this->cmd_line,arguments);
}

Command::~Command() {
    for (int j = 0; j < this->args_size; j++) {
        free(this->arguments[j]);
    }
    delete[] this->arguments;
    delete[] this->cmd_line;
}

SmallShell::SmallShell(): prompt("smash>"),jobs_list()  {
// TODO: add your implementation
    smash_pid= getpid();
    if (smash_pid == -1) // when pid didnt work
    {
        perror("smash error: getpid failed");
        return;
    }
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command(cmd_line) {};



/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0) {
      return new PromptCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
      return new ChangeDirCommand(cmd_line);
  }
  else if (firstWord.compare("jobs") == 0)
  {
      return new JobsCommand(cmd_line, &(this->jobs_list));
  }
 else if (firstWord.compare("quit") == 0) {
      return new QuitCommand(cmd_line);
  }

  else if (firstWord.compare("fg") == 0)
  {
      return new ForegroundCommand(cmd_line, &(this->jobs_list));
  }
  else if (firstWord.compare("bg") == 0)
  {
      return new BackgroundCommand(cmd_line, &(this->jobs_list));
  }


  else {
      return new ExternalCommand(cmd_line);
  }
//  else if ...
//  .....
//  else {
//    return new ExternalCommand(cmd_line);
//  }

  return nullptr;
}


void convertCmdLineToArgs(string cmd_line, vector<string>* arguments){
    const char* command = cmd_line.c_str();
    char** args_from_cmd = new char*[MAX_ARGS_IN_CMD]; //max of 20 args
    for(int i=0;i<MAX_ARGS_IN_CMD;++i){
        args_from_cmd[i] = new char();
        args_from_cmd[i] = nullptr;
    }
    _parseCommandLine(command,args_from_cmd);
    for(int i=0;i<MAX_ARGS_IN_CMD;++i){
        if(args_from_cmd[i]!= nullptr){
            arguments->push_back(args_from_cmd[i]);
        }
    }
    delete[] args_from_cmd;
}


void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    jobs_list.removeFinishedJobs();
    ///JobsList::removeFinishedTimes();
    Command *cmd = CreateCommand(cmd_line);
    if (cmd == nullptr) {
        return;
    }
    cmd->execute();
}



PromptCommand::PromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

void PromptCommand::execute() {
    string updated_name = "smash";
    if (this->args_size > 1) {
        updated_name = this->arguments[1];
    }
    updated_name.append(">");
    SmallShell & shell = SmallShell::getInstance();
    shell.prompt = updated_name;
}

ShowPidCommand::ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
void ShowPidCommand::execute() {
    SmallShell & shell = SmallShell::getInstance();
    if(shell.smash_pid >= 0)
    {
        cout << "smash pid is " << shell.smash_pid << endl;
        cout << "new name is: " << shell.prompt <<endl;

    }
    else
    {
        perror("smash error: getpid failed");
    }
    return;
}

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
void GetCurrDirCommand::execute() {
    char path[MAX_COMMAND_LENGTH];
    getcwd(path, sizeof(path));
    cout << path << endl;
    return;
}



ChangeDirCommand::ChangeDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

void ChangeDirCommand::execute()
{
    int result = 0;
    char current_dir[MAX_COMMAND_LENGTH];
    getcwd(current_dir, sizeof(current_dir));
    if (this->args_size > 2)
    {
        cerr <<"smash error: cd: too many arguments" << endl;
    }
    else
    {
        if(this->args_size == 2)
        {
            if (strcmp(this->arguments[1], "-") == 0) {
                if (ChangeDirCommand::path_history.empty()) { //can't find previous directory
                    cerr <<"smash error: cd: OLDPWD not set" << endl;
                }
                else
                {
                    result = chdir(ChangeDirCommand::path_history.back().c_str()); //prev directory is in the end of list
                    if (result == 0)
                    {
                        ChangeDirCommand::path_history.pop_back(); //now prev dir is update
                    }
                }
            }
            else {
                result = chdir(this->arguments[1]); // new directory is
                ChangeDirCommand::path_history.push_back(current_dir);
                if (result != 0) {
                    // you have an error :(
                    cerr <<"smash error: cd: chdir failed" << endl;
                }
            }
        }
    }
}

JobsList::JobEntry::JobEntry(int j_id, int j_process_id, Command* j_command, bool is_stopped, time_t time_job):
    j_id(j_id), j_process_id(j_process_id), j_command(j_command),
    is_stopped(is_stopped), time_job(time_job) {}

JobsList::JobsList() : jobs_list({}), counter(0){}


int JobsList::getHighestJob()
{
    int max = -1;
    auto it = this->jobs_list.begin();
    while (it != this->jobs_list.end()) {
        if(it->j_id > max)
        {
            max = it->j_id;
        }
        it++;
    }
    return max;
}

void JobsList::addJob(Command *cmd, int pid, bool isStopped) {
    JobsList::removeFinishedJobs();
    int job_highest = JobsList::getHighestJob();
    job_highest++;
    jobs_list.push_back(JobEntry(job_highest, pid, cmd,isStopped));
}

JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobs):
        BuiltInCommand(cmd_line), jobs_list(jobs) {}


void JobsCommand::execute()
{
    /////note that job list need to be updated before calling this function
    this->jobs_list->removeFinishedJobs();
    this->jobs_list->printJobsList();
}

void JobsList::removeFinishedJobs() {
    int stat;
    auto it = this->jobs_list.begin();
    while (it != this->jobs_list.end()) {
        int pid = it->j_process_id;
        if (waitpid(pid, &stat, WNOHANG) > 0 || kill(pid, 0) == -1) {
            delete it->j_command;
            this->counter--;
            it = this->jobs_list.erase(it);
        }
        it++;
    }
    if(jobs_list.size() == 0)
    {
        counter = 1;
    }
}

void JobsList::printJobsList() {
    JobsList::removeFinishedJobs();
    int job_id =-1;
    string command ="";
    int p_id = -1;
    time_t time_passed = 0;
    for (unsigned int i = 0; i < this->jobs_list.size(); i++) {
        job_id = JobsList::jobs_list[i].j_id;
        command = JobsList::jobs_list[i].j_command->cmd_line;
        p_id= JobsList::jobs_list[i].j_process_id;
        time_passed = difftime(time(0), JobsList::jobs_list[i].time_job);
        bool isStopped = JobsList::jobs_list[i].is_stopped;
        if (isStopped)
        {
            cout << "[" << job_id << "] " << command << " : " << p_id << " " << time_passed <<  " (stopped)" << endl;
        }
        else
        {

        }
        cout << "[" << job_id << "] " << command << " : " << p_id << " " << time_passed << endl;
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId)
{
    if (jobId <= 0 || jobs_list.empty() || jobId > this->counter) {
        return nullptr;
    }
    for(unsigned int i = 0; i < jobs_list.size(); i++)
    {
        if(jobs_list[i].j_id == jobId)
        {
            return &(jobs_list[i]);
        }
    }
    return nullptr;
}

JobsList::JobEntry* JobsList::getJobByPID(int j_process_id)
{
    if (j_process_id <= 0 || jobs_list.empty() || j_process_id > this->counter) {
        return nullptr;
    }
    for(unsigned int i = 0; i < jobs_list.size(); i++)
    {
        if(jobs_list[i].j_process_id == j_process_id)
        {
            return &(jobs_list[i]);
        }
    }
    return nullptr;
}



JobsList::JobEntry* JobsList::getLastJob() {
    int highest_job_id = getHighestJob();
    for (unsigned int i = 0; i < jobs_list.size(); i++) {
        if (jobs_list[i].j_id == highest_job_id) {
            return &jobs_list[i];

        }
    }
    return nullptr;
}

void JobsList::killAllJobs() {
    this->removeFinishedJobs();
    for (vector<JobEntry>::iterator i = jobs_list.begin(); i != jobs_list.end(); i++) {
        cout << i->j_process_id << ": " << i->j_command << endl;
        if (kill(i->j_process_id, SIGKILL) == -1) {
            perror("smash error: kill failed");
            return;
        }
        else
        {
            delete i->j_command;
            jobs_list.erase(i);
        }
    }
}


JobsList::JobEntry * JobsList::getLastStoppedJob(int *jobId) {
    int max_id_and_stopped = -1;
    for (unsigned int i = 0; i < jobs_list.size(); i++) {
        if ((jobs_list[i].j_id > max_id_and_stopped) && (jobs_list[i].is_stopped)) {
            max_id_and_stopped = jobs_list[i].j_id;
        }
    }
    if (max_id_and_stopped == -1) {
        return nullptr;
    } else {
        return getJobById(max_id_and_stopped);
    }
}


void JobsList::removeJobById(int jobId) {
    if (jobId <= 0 || jobs_list.empty() || jobId > this->counter)
        return;
    for (vector<JobEntry>::iterator i = jobs_list.begin(); i != jobs_list.end(); i++) {

        if (i->j_id == jobId) {
            delete i->j_command;
            jobs_list.erase(i);
            return;
        }
    }
    return;
}


void JobsList::printJobsListKilled()
{
    auto it = this->jobs_list.begin();
    while (it!=this->jobs_list.end()) {
        cout << it->j_id << ": " << it->j_command->cmd_line << endl;
        it++;

    }
}


ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobs):
        BuiltInCommand(cmd_line), jobs_list(jobs){}


void ForegroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    JobsList::JobEntry *selectedJob;
    if (this->args_size > 2) {
        perror("smash error: fg: invalid arguments");
        return;

    }
    if (this->args_size == 2) {
        selectedJob = this->jobs_list->getJobById(stoi(this->arguments[1]));
        if (selectedJob == nullptr) {
            string error = "smash error: fg: job-id ";
            error.append(this->arguments[1]);
            error.append(" does not exist");
            perror(error.c_str());
            return;

        }
    }
    else {
        selectedJob = this->jobs_list->getLastJob();
        if (selectedJob == nullptr)
            {
                cerr << "smash error: fg: jobs list is empty"<<endl;
                return;
            }
    }
    selectedJob = this->jobs_list->getJobById(stoi(this->arguments[1]));
    cout << *(selectedJob->j_command->cmd_line) << " : " << selectedJob->j_process_id << endl;

    if (selectedJob->is_stopped) {
        int res = kill(selectedJob->j_process_id, SIGCONT);
        if (res != 0) {
            perror("smash error: kill failed");
            exit(1);
        }
        selectedJob->is_stopped = false;
    }
    int pid_selected_job = selectedJob->j_process_id;
    smash.current_fg_pid = pid_selected_job;
    int status;
    int pid_to_run = selectedJob->j_process_id;
    if(waitpid(pid_selected_job , &status, WUNTRACED) == -1)
    {
        perror("smash error: waitpid failed");
        return;
    }
    jobs_list->removeJobById(selectedJob->j_id);

}



BackgroundCommand::BackgroundCommand(const char* cmd_line, JobsList* jobs)
        : BuiltInCommand(cmd_line), jobs_list(jobs) {}

void BackgroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    JobsList::JobEntry *selectedJob;
    if (this->args_size > 2) {
        perror("smash error: bg: invalid arguments");
        return;
    }
    if (this->args_size == 2) {
        int jobId = atoi(this->arguments[1]);
        selectedJob = JobsList::getJobById(jobId);
        if (selectedJob == nullptr) {
            string error = "smash error: bg: job-id ";
            error.append(this->arguments[1]);
            error.append(" does not exist");
            perror(error.c_str());
            return;
        }
    }


    int res = kill(selectedJob->j_process_id, SIGCONT);
    if (res == -1) {
        cout << "smash dont kill!!!!!!" << endl;
        exit(1);
    }
    selectedJob->is_stopped = false;
    return;

}



QuitCommand::QuitCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}


void QuitCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    smash.jobs_list.removeFinishedJobs();
    if (strcmp(arguments[1], "kill") == 0 && this->args_size > 1)
    {
        cout << "smash: sending SIGKILL signal to " << smash.jobs_list.getJobsListSize() <<" jobs:" <<endl;
        smash.jobs_list.killAllJobs();
    }
    exit(0);
}

int  JobsList::getJobsListSize()
{
    int counter =0;
    auto it = this->jobs_list.begin();
    while (it!=this->jobs_list.end()) {
        counter++;
        it++;
    }
    return counter;
}


ExternalCommand::ExternalCommand(const char* cmd_line): Command(cmd_line){}

void ExternalCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    pid_t pid = fork(); // here we fork the current procces
    if (pid == 0) { // son
        setpgrp();
        if (execvp(this->arguments[0], this->arguments) == -1) {
            perror("smash error: execvp failed");
        }
        exit(0);
    }
    else
    {
        smash.jobs_list.addJob(this,pid,false);
        int job_pid = smash.jobs_list.getLastJob()->j_id;

    }

    // after fork check ppid==0 i am doing setpgrp() and then i am exce him if father i am get pid=son and puting father on wait and into job list
}

RedirectionCommand::RedirectionCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void RedirectionCommand::execute() {

}

PipeCommand::PipeCommand(const char *cmd_line): Command(cmd_line)
{
//    std::string cmd_to_string = string(cmd_line);
//
//    if((cmd_to_string).find("|&") != std::string::npos)
//    {
//        symbol = "|&";
//        second_command = cmd_to_string.substr(cmd_to_string.find(symbol)+2,cmd_to_string.length());
//    }
//    else
//    {
//        symbol = "|";
//        second_command = cmd_to_string.substr(cmd_to_string.find(symbol)+1,cmd_to_string.length());
//
//    }
//    first_command = cmd_to_string.substr(0,cmd_to_string.find(symbol));
}

void PipeCommand::execute() {

}

void SmallShell::handle_alarm() {}





/*












*/