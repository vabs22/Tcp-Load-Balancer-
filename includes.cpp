#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define MAXSERVERS 70
#define MAXTAILNODES 257
#define MAXNETWORKS 1000
#define NUMCONNECTIONS 100
#define IPFORCODE	1729			// C.P. Ramanujam's number
#define ROUNDROBCODE 151031			// A.P.J Abdul kalam sir's birth date 
#define MAXWEIGHT 256

const int MAXROUNDROBSLOTS = MAXSERVERS*MAXWEIGHT + 10; 

using namespace std;

