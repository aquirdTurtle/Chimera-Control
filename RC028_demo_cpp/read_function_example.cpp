/*
 * WARNING!!!
 * This example wasn't compiled or tested, but should show the strategy to read from the FPGA.
*/

int RC028::read_running()
{
    --variables for write
    FT_STATUS ftStatus;
    DWORD BytesWritten;
    unsigned char dataBuffer[7];
    unsigned long dwNumberOfBytesSent = 0;
    --variables for read
    DWORD bytestoread = 7
    DWORD BytesRead = 1;
    DWORD TotalBytesRead = 0;

    while(BytesRead != 0)
    {
        ftStatus = FT_Read(this->ftHandle, dataBuffer, DWORD(7), &BytesRead);
        if (ftStatus != FT_OK) {
            // FT_Write Failed
            cout << "error reading" << endl;
            return 1;
        }
    }

    //letting fpga know we want it to send us value in address 0x0003
    dataBuffer[0] = 129;
    dataBuffer[1] = 0;
    dataBuffer[2] = 3;
    dataBuffer[3] = 0;
    dataBuffer[4] = 0;
    dataBuffer[5] = 0;
    dataBuffer[6] = 0;

    ftStatus = FT_Write(this->ftHandle, dataBuffer, sizeof(dataBuffer), &BytesWritten);
    if (ftStatus == FT_OK) {
        // FT_Write OK
        std::cout << "Bytes sent for read request: " << dwNumberOfBytesSent << endl;
    }
    else {
        // FT_Write Failed
        cout << "error writing" << endl;
        return 1;
    }
    
    while(TotalBytesRead < bytestoread)
    {
        ftStatus = FT_Read(this->ftHandle, dataBuffer, bytestoread, &BytesRead);
        if (ftStatus == FT_OK) {
            // FT_Write OK
            std::cout << "Bytes sent for read request: " << dwNumberOfBytesSent << endl;
            if(dataBuffer[6] == 1) {
                std::cout << "Sequencer running..." << endl;
            } else
                std::cout << "Sequencer NOT running..." << endl;
            }
            return 0;
        }
        else {
            // FT_Write Failed
            cout << "error writing" << endl;
            return 1;
        }
        TotalBytesRead += BytesRead;
    }

    return 1;
}
