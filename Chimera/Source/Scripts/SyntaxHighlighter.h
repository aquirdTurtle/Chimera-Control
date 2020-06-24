#pragma once

#include <Scripts/ScriptableDevices.h>
#include <qsyntaxhighlighter.h>
#include <qregularexpression.h>
#include <GeneralObjects/Matrix.h>
#include <string>
#include <ParameterSystem/ParameterSystemStructures.h>

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT 

    public:
        SyntaxHighlighter (ScriptableDevice device, QTextDocument* parent = 0);
        void setLocalParams (std::vector<parameterType> localParams);
        void setOtherParams (std::vector<parameterType> otherParams);
        void setTtlNames (Matrix<std::string> ttlNames);
        void setDacNames (std::vector<std::string> dacNames);

    protected:
        void highlightBlock (const QString& text) override;
        
    private:
        struct HighlightingRule {
            QRegularExpression pattern;
            QTextCharFormat format;
        };
        void addRules (QVector<QString> regexStrings, QColor color, bool bold, bool addWordReq);
        void addRules (QVector<QString> regexStrings, QColor color, bool bold, bool addWordReq, QVector<HighlightingRule>& rules);
        const ScriptableDevice deviceType;

        QVector<HighlightingRule> mainRules;
        QVector<HighlightingRule> aoRules;
        QVector<HighlightingRule> doRules;
        QVector<HighlightingRule> localParamRules;
        QVector<HighlightingRule> otherParamRules;

        QRegularExpression commentStartExpression;
        QRegularExpression commentEndExpression;

        QTextCharFormat multiLineCommentFormat;
};

