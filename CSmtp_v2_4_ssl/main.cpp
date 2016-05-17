#include "CSmtp.h"
#include <iostream>

int main()
{
	bool bError = false;
	
	try
	{
		CSmtp mail;
		#define test_gmail_tls
		#if defined(test_gmail_tls)
				mail.SetSMTPServer("smtp.gmail.com",587);
				mail.SetSecurityType(USE_TLS);
		#elif defined(test_gmail_ssl)
				mail.SetSMTPServer("smtp.gmail.com",465);
				mail.SetSecurityType(USE_SSL);
		#elif defined(test_hotmail_TLS)
				mail.SetSMTPServer("smtp.live.com",25);
				mail.SetSecurityType(USE_TLS);
		#elif defined(test_aol_tls)
				mail.SetSMTPServer("smtp.aol.com",587);
				mail.SetSecurityType(USE_TLS);
		#elif defined(test_yahoo_ssl)
				mail.SetSMTPServer("plus.smtp.mail.yahoo.com",465);
				mail.SetSecurityType(USE_SSL);
		#endif

		mail.SetLogin("Mark.O.Brown@colorado.edu");
		mail.SetPassword("<>XF3vYGs7cHgZ5");
  		mail.SetSenderName("Mark");
  		mail.SetSenderMail("Mark.O.Brown@colorado.edu");
  		mail.SetReplyTo("Mark.O.Brown@colorado.edu");
  		mail.SetSubject("");
  		mail.AddRecipient("7032544981@vzwpix.com");
  		mail.SetXPriority(XPRIORITY_NORMAL);
  		mail.SetXMailer("The Bat! (v3.02) Professional");
  		mail.AddMsgLine("Dark Souls 3 Awaits");
  		//mail.AddAttachment("../test1.jpg");
  		//mail.AddAttachment("c:\\test2.exe");
		//mail.AddAttachment("c:\\test3.txt");
		mail.Send();
	}
	catch(ECSmtp e)
	{
		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
		bError = true;
	}
	if(!bError)
		std::cout << "Mail was send successfully.\n";
	std::cin.get();
	return 0;
}
