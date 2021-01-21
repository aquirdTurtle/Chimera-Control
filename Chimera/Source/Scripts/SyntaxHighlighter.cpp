#include <stdafx.h>
#include <Scripts/SyntaxHighlighter.h>
#include <NIAWG/NiawgStructures.h>
#include <GeneralUtilityFunctions/my_str.h>
#include <qdebug.h>

SyntaxHighlighter::SyntaxHighlighter (ScriptableDevice device, QTextDocument* parent) : 
	QSyntaxHighlighter (parent), 
	deviceType(device){

	HighlightingRule rule;
	// math symbols
	addRules ({ "[\\+\\=\\{\\}\\(\\)\\*\\-\\/]" }, QColor (42, 161, 152), true, false);
    multiLineCommentFormat.setForeground (QColor(23, 84, 81));

	QTextCharFormat numberFormat;
	numberFormat.setForeground (QColor (255, 255, 255));
	rule.pattern = QRegularExpression (QStringLiteral ("[\.0-9]"));
	rule.format = numberFormat;
	mainRules.append (rule);

	QTextCharFormat functionFormat;
    functionFormat.setFontItalic (true);
    functionFormat.setForeground (Qt::cyan);
    rule.pattern = QRegularExpression (QStringLiteral ("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    mainRules.append (rule);
	
	QTextCharFormat localVarFormat;
	localVarFormat.setForeground (QColor (200, 200, 255));
	// this regex uses a "lookbehind" to match the *word* after the var declaration.
	rule.pattern = QRegularExpression (QStringLiteral ("(?<=\\bvar\\s)(\\w+)"));
	rule.format = localVarFormat;
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
		for (auto num : range (NiawgConstants::MAX_NIAWG_SIGNALS)) {
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
		addRules ({ "var_v", "var" }, QColor (42, 161, 152), true, false);
		addRules ({ "#" }, QColor (100, 100, 100), true, false);
	}
	else if (device == ScriptableDevice::Agilent) {
		addRules ({ "var" }, QColor (42, 161, 152), true, false);
		addRules ({"ramp", "hold", "pulse"}, QColor(108, 113, 196), true, true);
		addRules ({ "once", "oncewaittrig", "lin", "tanh", "repeatuntiltrig" ,"repeat", "sech" , "gaussian", "lorentzian" }, 
					QColor (181, 137, 0), true, true);
		addRules ({ "#" }, QColor (100, 100, 100), true, false);
	}

	QTextCharFormat singleLineCommentFormat;
	singleLineCommentFormat.setForeground (QColor (101, 115, 126));
	rule.pattern = QRegularExpression (QStringLiteral ("%[^\n]*"));
	rule.format = singleLineCommentFormat;
	mainRules.append (rule);
}

void SyntaxHighlighter::setTtlNames (Matrix<std::string> ttlNames) {
	QVector<QString> doNamesRegex;
	for (auto rowInc : range (ttlNames.getRows ())) {
		for (auto num : range (ttlNames.getCols ())) {
			auto rowStr = std::vector<std::string>{ "a", "b", "c", "d" }[rowInc];
			doNamesRegex.push_back (cstr (rowStr + str (num)));
			doNamesRegex.push_back (cstr (ttlNames (rowInc, num)));
		}
	}
	doRules.clear ();
	addRules (doNamesRegex, QColor (200, 200, 0), false, true, doRules);
}

void SyntaxHighlighter::setDacNames (std::vector<std::string> dacNames) {
	QVector<QString> aoNamesRegex;
	for (auto dacInc : range (dacNames.size ())) {
		aoNamesRegex.push_back (cstr ("dac" + str (dacInc)));
		aoNamesRegex.push_back (cstr (dacNames[dacInc]));
	}
	aoRules.clear ();
	addRules (aoNamesRegex, QColor (203, 75, 22), false, true, aoRules);
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
	addRules (names, QColor (200, 200, 255), true, true, localParamRules);
}

void SyntaxHighlighter::setOtherParams (std::vector<parameterType> otherParams) {
	QVector<QString> names;
	for (auto param : otherParams) {
		names.push_back (param.name.c_str ());
	}
	addRules (names, QColor (0, 255, 0), true, true, otherParamRules);
}

void SyntaxHighlighter::highlightBlock (const QString& text){
	// a lot of the logic here taken from the qt documentation example for this stuff.  The order of the rules and their
	// application here determines which highlighting rules take precedence. 
	for (const HighlightingRule& rule : qAsConst (doRules)) {
		auto matchIterator = rule.pattern.globalMatch (text);
		while (matchIterator.hasNext ()) {
			auto match = matchIterator.next ();
			setFormat (match.capturedStart (), match.capturedLength (), rule.format);
		}
	}
	for (const HighlightingRule& rule : qAsConst (aoRules)) {
		auto matchIterator = rule.pattern.globalMatch (text);
		while (matchIterator.hasNext ()) {
			auto match = matchIterator.next ();
			setFormat (match.capturedStart (), match.capturedLength (), rule.format);
		}
	}
	for (const HighlightingRule& rule : qAsConst (localParamRules)) {
		auto matchIterator = rule.pattern.globalMatch (text);
		while (matchIterator.hasNext ()) {
			auto match = matchIterator.next ();
			setFormat (match.capturedStart (), match.capturedLength (), rule.format);
		}
	}
	for (const HighlightingRule& rule : qAsConst (otherParamRules)) {
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch (text);
		while (matchIterator.hasNext ()) {
			auto match = matchIterator.next ();
			setFormat (match.capturedStart (), match.capturedLength (), rule.format);
		}
	}
	for (const HighlightingRule& rule : qAsConst (mainRules)) {
		auto matchIterator = rule.pattern.globalMatch (text);
		while (matchIterator.hasNext ()) {
			auto match = matchIterator.next ();
			setFormat (match.capturedStart (), match.capturedLength (), rule.format);
			//qDebug () << rule;
			qDebug () << rule.pattern;
			qDebug () << rule.format;
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
            commentLength = endIndex - startIndex + match.capturedLength ();
        }
        setFormat (startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf (commentStartExpression, startIndex + commentLength);
    }
}
