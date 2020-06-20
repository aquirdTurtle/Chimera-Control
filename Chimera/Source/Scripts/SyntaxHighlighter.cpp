#include <stdafx.h>
#include <Scripts/SyntaxHighlighter.h>

SyntaxHighlighter::SyntaxHighlighter (ScriptableDevice device, QTextDocument* parent) : 
	QSyntaxHighlighter (parent), 
	deviceType(device){
    
	//// convert word to lower case.
	//std::transform (word.begin (), word.end (), word.begin (), ::tolower);

	//for (const auto& var : params)
	//{
	//	if (word == var.name)
	//	{
	//		return _myRGBs["Solarized Green"];
	//	}
	//}
	//for (const auto& var : localParams)
	//{
	//	if (word == var.name)
	//	{
	//		return _myRGBs["Solarized Blue"];
	//	}
	//}
	HighlightingRule rule;
	addRules ({ "[\\+\\=\\(\\)\\*\\-\\/]" }, QColor (42, 161, 152), true, false);

	QTextCharFormat singleLineCommentFormat;
	singleLineCommentFormat.setForeground (QColor (101, 115, 126));
	rule.pattern = QRegularExpression (QStringLiteral ("%[^\n]*"));
    rule.format = singleLineCommentFormat;
    mainRules.append (rule);

    multiLineCommentFormat.setForeground (QColor(23, 84, 81));

	QTextCharFormat functionFormat;
    functionFormat.setFontItalic (true);
    functionFormat.setForeground (Qt::blue);
    rule.pattern = QRegularExpression (QStringLiteral ("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    mainRules.append (rule);

    commentStartExpression = QRegularExpression (QStringLiteral ("/\\*"));
    commentEndExpression = QRegularExpression (QStringLiteral ("\\*/"));

	if (device == ScriptableDevice::Master) {
		addRules ({ "on","off","pulseon","pulseoff" }, QColor (42, 161, 152), true, true);
		addRules ({ "dac","dacarange","daclinspace", "repeat", "end", "callcppcode",
					"loadskipentrypoint!" }, QColor (42, 161, 152), true, true);
		addRules ({ "call", "def" }, QColor (38, 139, 210), true, true);
		addRules ({ "t" }, QColor (255, 255, 255), false, true);
		addRules ({ ":" }, QColor (255, 255, 255), false, false);
		addRules ({ "sin","cos","tan","exp","ln","var" }, QColor (42, 161, 152), true, true);



	}
	else if (device == ScriptableDevice::NIAWG) {
		QVector<QString> niawgCommands = { "flash", "rearrange", "horizontal", "vertical" };
		for (auto num : range (MAX_NIAWG_SIGNALS)) {
			niawgCommands.push_back (cstr ("gen" + str (num + 1) + "const"));
			niawgCommands.push_back (cstr ("gen" + str (num + 1) + "ampramp"));
			niawgCommands.push_back (cstr ("gen" + str (num + 1) + "freqramp"));
			niawgCommands.push_back (cstr ("gen" + str (num + 1) + "freq&ampramp"));
			niawgCommands.push_back (cstr ("gen" + str (num + 1) + "const_v"));
			niawgCommands.push_back (cstr ("gen" + str (num + 1) + "ampramp_v"));
			niawgCommands.push_back (cstr ("gen" + str (num + 1) + "freqramp_v"));
			niawgCommands.push_back (cstr ("gen" + str (num + 1) + "freq&ampramp_v"));
		}
		addRules (niawgCommands, QColor (108, 113, 196), true, true);
		addRules ({"repeattiltrig", "repeatset#", "repeattilsoftwaretrig", "endrepeat", "repeatforever"}, 
				  QColor (38, 139, 210), true, true);
		addRules ({"lin", "nr", "tanh"}, QColor(133, 153, 0), true, true);
		addRules ({ "\\{","\\}","\\[","\\]" }, QColor(42, 161, 152),true,false);
		addRules ({ "var_v" }, QColor (42, 161, 152), true, false);
		addRules ({ "#" }, QColor (100, 100, 100), true, false);
	}
	else if (device == ScriptableDevice::Agilent) {
		addRules ({"ramp", "hold", "pulse"}, QColor(108, 113, 196), true, true);
		addRules ({ "once", "oncewaittrig", "lin", "tanh", "repeatuntiltrig" ,"repeat", "sech" , "gaussian", "lorentzian" }, 
					QColor (181, 137, 0), true, true);
		addRules ({ "#" }, QColor (100, 100, 100), true, false);
	}

	QTextCharFormat numberFormat;
	numberFormat.setForeground (QColor (255, 255, 255));
	rule.pattern = QRegularExpression (QStringLiteral ("[\.0-9]"));
	rule.format = numberFormat;
	mainRules.append (rule);
}

void SyntaxHighlighter::setTtlNames (Matrix<std::string> ttlNames) {
	QVector<QString> ttlNamesRegex;
	for (auto rowInc : range (ttlNames.getRows ())) {
		for (auto num : range (ttlNames.getCols ())) {
			auto rowStr = std::vector<std::string>{ "a", "b", "c", "d" }[rowInc];
			ttlNamesRegex.push_back (cstr (rowStr + str (num)));
			ttlNamesRegex.push_back (cstr (ttlNames (rowInc, num)));
		}
	}
	doRules.clear ();
	addRules (ttlNamesRegex, QColor (42, 161, 152), false, true, doRules);
}

void SyntaxHighlighter::setDacNames (std::vector<std::string> dacNames) {
	QVector<QString> dacNamesRegex;
	for (auto dacInc : range (dacNames.size ())) {
		dacNamesRegex.push_back (cstr ("dac" + str (dacInc)));
		dacNamesRegex.push_back (cstr (dacNames[dacInc]));
	}
	aoRules.clear ();
	addRules (dacNamesRegex, QColor (203, 75, 22), false, true, aoRules);
}

void SyntaxHighlighter::addRules (QVector<QString> regexStrings, QColor color, bool bold, bool addWordReq) {
	addRules (regexStrings, color, bold, addWordReq, mainRules);
}
void SyntaxHighlighter::addRules (QVector<QString> regexStrings, QColor color, bool bold, bool addWordReq, QVector<HighlightingRule>& rules) {
	QTextCharFormat format;
	format.setForeground (color);
	if (bold) {
		format.setFontWeight (QFont::Bold);
	}
	if (addWordReq) {
		for (auto& kw : regexStrings) {
			kw = "\\b" + kw + "\\b";
		}
	}
	HighlightingRule rule;
	for (const QString& pattern : regexStrings) {
		rule.pattern = QRegularExpression (pattern, QRegularExpression::PatternOption::CaseInsensitiveOption);
		rule.format = format;
		rules.append (rule);
	}
}

void SyntaxHighlighter::setLocalParams (std::vector<parameterType> localParams) {
	QVector<QString> names;
	for (auto param : localParams) {
		names.push_back (param.name.c_str());
	}
	addRules (names, QColor (0, 255, 0), true, true, localParamRules);
}

void SyntaxHighlighter::setOtherParams (std::vector<parameterType> otherParams) {
	QVector<QString> names;
	for (auto param : otherParams) {
		names.push_back (param.name.c_str ());
	}
	addRules (names, QColor (0, 255, 0), true, true, otherParamRules);
}

void SyntaxHighlighter::highlightBlock (const QString& text){
    for (const HighlightingRule& rule : qAsConst (mainRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch (text);
        while (matchIterator.hasNext ()) {
            QRegularExpressionMatch match = matchIterator.next ();
            setFormat (match.capturedStart (), match.capturedLength (), rule.format);
        }
    }
	for (const HighlightingRule& rule : qAsConst (doRules)) {
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch (text);
		while (matchIterator.hasNext ()) {
			QRegularExpressionMatch match = matchIterator.next ();
			setFormat (match.capturedStart (), match.capturedLength (), rule.format);
		}
	}
	for (const HighlightingRule& rule : qAsConst (aoRules)) {
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch (text);
		while (matchIterator.hasNext ()) {
			QRegularExpressionMatch match = matchIterator.next ();
			setFormat (match.capturedStart (), match.capturedLength (), rule.format);
		}
	}
    setCurrentBlockState (0);

    int startIndex = 0;
	if (previousBlockState () != 1) {
		startIndex = text.indexOf (commentStartExpression);
	}

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match (text, startIndex);
        int endIndex = match.capturedStart ();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState (1);
            commentLength = text.length () - startIndex;
        }
        else {
            commentLength = endIndex - startIndex
                + match.capturedLength ();
        }
        setFormat (startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf (commentStartExpression, startIndex + commentLength);
    }
}
