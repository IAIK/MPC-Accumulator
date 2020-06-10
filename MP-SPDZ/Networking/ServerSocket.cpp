/*
 * ServerSocket.cpp
 *
 */

#include <Networking/ServerSocket.h>
#include <Networking/sockets.h>
#include "Exceptions/Exceptions.h"

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
using namespace std;

void* accept_thread(void* server_socket)
{
  ((ServerSocket*)server_socket)->accept_clients();
  return 0;
}

ServerSocket::ServerSocket(int Portnum) : portnum(Portnum), thread(0)
{
  struct sockaddr_in serv; /* socket info about our server */

  memset(&serv, 0, sizeof(serv));    /* zero the struct before filling the fields */
  serv.sin_family = AF_INET;         /* set the type of connection to TCP/IP */
  serv.sin_addr.s_addr = INADDR_ANY; /* set our address to any interface */
  serv.sin_port = htons(Portnum);    /* set the server port number */

  main_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (main_socket<0) { error("set_up_socket:socket"); }

  int one=1;
  int fl=setsockopt(main_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&one,sizeof(int));
  if (fl<0) { error("set_up_socket:setsockopt"); }

  /* disable Nagle's algorithm */
  fl= setsockopt(main_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&one,sizeof(int));
  if (fl<0) { error("set_up_socket:setsockopt");  }

  octet my_name[512];
  memset(my_name,0,512*sizeof(octet));
  gethostname((char*)my_name,512);

  /* bind serv information to mysocket
   *   - Just assume it will eventually wake up
   */
  fl=1;
  while (fl!=0)
    { fl=::bind(main_socket, (struct sockaddr *)&serv, sizeof(struct sockaddr));
      if (fl != 0)
        { cerr << "Binding to socket on " << my_name << ":" << Portnum << " failed, trying again in a second ..." << endl;
          sleep(1);
        }
#ifdef DEBUG_NETWORKING
      else
        { cerr << "ServerSocket is bound on port " << Portnum << endl; }
#endif
    }
  if (fl<0) { error("set_up_socket:bind");  }

  /* start listening, allowing a queue of up to 1000 pending connection */
  fl=listen(main_socket, 1000);
  if (fl<0) { error("set_up_socket:listen");  }

  // Note: must not call virtual init() method in constructor: http://www.aristeia.com/EC3E/3E_item9.pdf
}

void ServerSocket::init()
{
  pthread_create(&thread, 0, accept_thread, this);
}

ServerSocket::~ServerSocket()
{
  pthread_cancel(thread);
  pthread_join(thread, 0);
  if (close(main_socket)) { error("close(main_socket"); };
}

void ServerSocket::accept_clients()
{
  while (true)
    {
      struct sockaddr dest;
      memset(&dest, 0, sizeof(dest));    /* zero the struct before filling the fields */
      int socksize = sizeof(dest);
      int consocket = accept(main_socket, (struct sockaddr *)&dest, (socklen_t*) &socksize);
      if (consocket<0) { error("set_up_socket:accept"); }

      int client_id;
      try
      {
          receive(consocket, (unsigned char*)&client_id, sizeof(client_id));
      }
      catch (closed_connection&)
      {
#ifdef DEBUG_NETWORKING
          auto& conn = *(sockaddr_in*)&dest;
          cerr << "client on " << inet_ntoa(conn.sin_addr) << ":"
                  << ntohs(conn.sin_port) << " left without identification"
                  << endl;
#endif
      }

      data_signal.lock();
      process_client(client_id);
      clients[client_id] = consocket;
      data_signal.broadcast();
      data_signal.unlock();

#ifdef __APPLE__
      int flags = fcntl(consocket, F_GETFL, 0);
      int fl = fcntl(consocket, F_SETFL, O_NONBLOCK |  flags);
      if (fl < 0)
          error("set non-blocking");
#endif
    }
}

int ServerSocket::get_connection_count()
{
  data_signal.lock();
  int connection_count = clients.size();
  data_signal.unlock();
  return connection_count;
}


int ServerSocket::get_connection_socket(int id)
{
  data_signal.lock();
  if (used.find(id) != used.end())
    {
      stringstream ss;
      ss << "Connection id " << hex << id << " already used";
      throw IO_Error(ss.str());
    }

  while (clients.find(id) == clients.end())
  {
      if (data_signal.wait(60) == ETIMEDOUT)
          throw runtime_error("No client after one minute");
  }

  int client_socket = clients[id];
  used.insert(id);
  data_signal.unlock();
  return client_socket;
}

void* anonymous_accept_thread(void* server_socket)
{
  ((AnonymousServerSocket*)server_socket)->accept_clients();
  return 0;
}

void AnonymousServerSocket::init()
{
  pthread_create(&thread, 0, anonymous_accept_thread, this);
}

int AnonymousServerSocket::get_connection_count()
{
  return num_accepted_clients;
}

void AnonymousServerSocket::process_client(int client_id)
{
  if (clients.find(client_id) != clients.end())
    close_client_socket(clients[client_id]);
  num_accepted_clients++;
  client_connection_queue.push(client_id);
}

int AnonymousServerSocket::get_connection_socket(int& client_id)
{
  data_signal.lock();

  //while (clients.find(next_client_id) == clients.end())
  while (client_connection_queue.empty())
      data_signal.wait();

  client_id = client_connection_queue.front();
  client_connection_queue.pop();
  int client_socket = clients[client_id];
  data_signal.unlock();
  return client_socket;
}
