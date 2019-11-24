#include "stdafx.h"
#include "SocketWrapper.h"

void SocketWrapper::initialize()
{
	if (!SOCKET_SAFEMODE)
	{
		if (server.Create( 10010, SOCK_STREAM ) == 0)
		{
			thrower( "Socket initialization error!" );
		}
	}
}

/// Get accumulations from master. Procedure is
// this computer >> Other Computer ("Acumulations?")
// Other Computer >> this computer ("Accumulations: (#)")
// this computer >> other computer ("Received Accumulations.")
int SocketWrapper::getRepetitions(int scalingFactor)
{
	int num = 0;
	send( "Accumulations?" );
	std::string recievedMsg = recieve();
	if (!NIAWG_SAFEMODE)
	{
		std::string tempRepetitions;
		std::stringstream repetitionStream;
		repetitionStream << recievedMsg;
		repetitionStream >> tempRepetitions;
		if (tempRepetitions != "Accumulations:")
		{
			thrower( "ERROR: master's message did not start with \"Accumulations:\". It started with "
					 + tempRepetitions + " . Assuming fatal error." );
		}
		repetitionStream >> tempRepetitions;
		try
		{
			num = std::stoi( tempRepetitions );
		}
		catch (std::invalid_argument&)
		{
			thrower( "ERROR: master's message's number did not convert correctly to an integer. String trying to convert is "
					 + tempRepetitions + ". Assuming fatal error." );
		}
		if (num < 0)
		{
			thrower( "ERROR: master's message's number was negative! String trying to convert is " + tempRepetitions
					 + ". Assuming fatal error." );
		}
		send( "Recieved Accumulations." );
		if (num % scalingFactor != 0)
		{
			thrower( "ERROR: Number of accumulations received from master: " + str( num )
					 + ", is not an integer multiple of the number of configurations in the sequence: "
					 + str( scalingFactor ) + ". It must be." );
		}
	}
	else
	{
		return 100;
	}
	return num / scalingFactor;
}

void SocketWrapper::connect()
{
	if (!SOCKET_SAFEMODE)
	{
		if (!client.Connect( SERVER_ADDRESS, std::stoi(DEFAULT_PORT) ))
		{
			thrower( "Socket connect failed!" );
		}
	}
	connectedStatus = true;
}


void SocketWrapper::send(std::string message)
{
	if (!SOCKET_SAFEMODE)
	{
		if (client.Send( cstr(message), message.size() ) == SOCKET_ERROR)
		{
			thrower( "ERROR: socket send error" );
		}
	}
}

bool SocketWrapper::isConnected()
{
	return connectedStatus;
}

std::string SocketWrapper::recieve()
{
	//std::string message;
	char message[256];
	if (!SOCKET_SAFEMODE)
	{
		if (client.Receive( message, 256 ) == SOCKET_ERROR)
		{
			thrower( "ERROR: Socket recieve error!" );
		}
		return message;
	}
	else
	{
		return "100";
	}
}


void SocketWrapper::close()
{
	// no return value for Close()... for some reason T.T
	if (!SOCKET_SAFEMODE)
	{
		client.Close();
	}
	connectedStatus = false;
}

