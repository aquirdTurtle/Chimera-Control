#include "stdafx.h"

#include "easendmailobj.tlh"
#include <iostream>

int _tmain(int argc, _TCHAR* argv[])
{
	::CoInitialize(NULL);

	EASendMailObjLib::IMailPtr oSmtp = NULL;
	oSmtp.CreateInstance("EASendMailObj.Mail");
	oSmtp->LicenseCode = _T("TryIt");

	// Set your sender email address
	oSmtp->FromAddr = _T("o.mark.brown@gmail.com");

	// Add recipient email address
	oSmtp->AddRecipientEx(_T("7032544981@vzwpix.com"), 0);

	// Set email subject
	oSmtp->Subject = _T("");

	// Set email body
	oSmtp->BodyText = _T("LASER UNLOCKED.");

	// Your SMTP server address
	oSmtp->ServerAddr = _T("smtp.gmail.com");

	// User and password for ESMTP authentication, if your server doesn't 
	// require User authentication, please remove the following codes.
	oSmtp->UserName = _T("o.mark.brown@gmail.com");
	oSmtp->Password = _T("ruFF#nar5=cUeL<");

	// Set SSL 465 port
	oSmtp->ServerPort = 465;

	// Set direct SSL connection
	oSmtp->SSL_starttls = 0;
	oSmtp->SSL_init();

	_tprintf(_T("Start to send email ...\r\n"));

	if (oSmtp->SendMail() == 0)
	{
		_tprintf(_T("email was sent successfully!\r\n"));
	}
	else
	{
		_tprintf(_T("failed to send email with the following error: %s\r\n"),
			(const TCHAR*)oSmtp->GetLastErrDescription());
	}

	if (oSmtp != NULL)
		oSmtp.Release();
	std::cin.get();
	return 0;
}
