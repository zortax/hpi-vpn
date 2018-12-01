#include <iostream>
#include <algorithm>
#include <vector>
#include <termios.h>
#include <unistd.h>

using namespace std;

string username;
string password;
string host;

void setStdinEcho(bool enable = true) {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if (!enable)
	tty.c_lflag &= ~ECHO;
    else
	tty.c_lflag |= ECHO;
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

char* getCmdOption(char ** begin, char ** end, const string & option) {
    char ** itr = find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const string & option) {
    return find(begin, end, option) != end;
}

int removeHost(string host) {
    cout << "Removing \"" << host << "\"..." << endl;
    string cmd = "sudo sed -i \"/" + host + "/d\" /etc/hosts";
    string kill_ssh_cmd = "sudo pkill ssh";
    system(cmd.c_str());
    return system(kill_ssh_cmd.c_str());
}

int addHost(string host) {
    cout << "Adding \"" << host << "\"..." << endl;
    string cmd = "grep " + host + " /etc/hosts > /dev/null || echo \"127.0.0.1 " + host + "\" | sudo tee -a /etc/hosts";
    string ssh_cmd = "sudo sshpass -p \'" + password + "\' ssh -oStrictHostKeyChecking=no -oIdentitiesOnly=yes -f -N " + username + "@ssh-stud.hpi.uni-potsdam.de -L443:" + host + ":443";
    system(cmd.c_str());
    return system(ssh_cmd.c_str());
}

void discOnExit() {
    if (!host.empty())
	removeHost(host);
}

int main(int argc, char** argv) {
    atexit(discOnExit);
    if (cmdOptionExists(argv, argv+argc, "--help")) {
        cout << "--- Help ---" << endl;
	cout << " # hpi-vpn      Modiefies your /etc/hosts file and starts a SSH Tunnel to access the HPI intranet." << endl;
	cout << "--- Flags ---" << endl;
	cout << " -u <username>  Sets your HPI username and skips the interactive input." << endl;
	cout << " -p <password>  Sets your HPI password and skips the interactive input (not recommended)." << endl;
    }

    if (cmdOptionExists(argv, argv+argc, "-u")) {
        username = getCmdOption(argv, argv+argc, "-u");
    } else {
	cout << "HPI username: ";
	cin >> username;
    }

    if (cmdOptionExists(argv, argv+argc, "-p")) {
        username = getCmdOption(argv, argv+argc, "-p");
    } else {
	setStdinEcho(false);
        cout << "HPI password: ";
	cin >> password;
	setStdinEcho(true);
	cout << "    " << endl;
    }

    while (true) {
        cout << "Enter HPI-Host you want to connect to (q to quit): ";
	string h;
	cin >> h;
	if (!h.compare("q")) {
	    return 0;
	}
	if (!host.empty()) {
	    removeHost(host);
	}
	host = h;
	cout << "Connecting..." << endl;
	addHost(host);
	string cmd = "nohup xdg-open https://" + host + " &";
	system(cmd.c_str());
    }
    
    return 0;
}



