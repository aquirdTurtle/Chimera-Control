#pragma once
#include "Windows.h"
#include "Control.h"

class TextPromptDialog : public CDialog
{
	DECLARE_DYNAMIC(TextPromptDialog);

	public:

		TextPromptDialog::TextPromptDialog(std::string* resultPtr, std::string description, bool isPassword=false) 
			: CDialog(IDD_TEXT_PROMPT_DIALOG)
		{
			passwordOption = isPassword;
			result = resultPtr;
			descriptionText = description;
		}

		BOOL OnInitDialog() override;

		void catchOk();
		void catchCancel();

	private:	
		DECLARE_MESSAGE_MAP();
		std::string descriptionText;
		Control<CStatic> description;
		Control<CEdit> prompt;
		std::string* result;
		bool passwordOption;
};

