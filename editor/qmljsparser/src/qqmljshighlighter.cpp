#include "qqmljshighlighter_p.h"
#include <QDebug>

namespace lcv{

QQmlJsHighlighter::QQmlJsHighlighter(QTextDocument *parent, lcv::QDocumentCodeState *state)
    : QSyntaxHighlighter(parent)
    , m_documentState(state)
{
    m_formatRoles["text"]        = QQmlJsHighlighter::Text;
    m_formatRoles["comment"]     = QQmlJsHighlighter::Comment;
    m_formatRoles["number"]      = QQmlJsHighlighter::Number;
    m_formatRoles["string"]      = QQmlJsHighlighter::String;
    m_formatRoles["operator"]    = QQmlJsHighlighter::Operator;
    m_formatRoles["identifier"]  = QQmlJsHighlighter::Identifier;
    m_formatRoles["keyword"]     = QQmlJsHighlighter::Keyword;
    m_formatRoles["builtin"]     = QQmlJsHighlighter::BuiltIn;
    m_formatRoles["qmlproperty"] = QQmlJsHighlighter::QmlProperty;
    m_formatRoles["qmltype"]     = QQmlJsHighlighter::QmlType;
    m_formatRoles["qmlruntimeboundproperty"] = QQmlJsHighlighter::QmlRuntimeBoundProperty;
    m_formatRoles["qmlruntimemodifiedvalue"] = QQmlJsHighlighter::QmlRuntimeModifiedValue;
    m_formatRoles["qmledit"] = QQmlJsHighlighter::QmlEdit;

    m_formats[QQmlJsHighlighter::Text]        = createFormat("#fff");
    m_formats[QQmlJsHighlighter::Comment]     = createFormat("#56748a");
    m_formats[QQmlJsHighlighter::Number]      = createFormat("#ca7000");
    m_formats[QQmlJsHighlighter::String]      = createFormat("#358d37");
    m_formats[QQmlJsHighlighter::Operator]    = createFormat("#c0a000");
    m_formats[QQmlJsHighlighter::Identifier]  = createFormat("#93672f");
    m_formats[QQmlJsHighlighter::Keyword]     = createFormat("#a0a000");
    m_formats[QQmlJsHighlighter::BuiltIn]     = createFormat("#93672f");

    m_formats[QQmlJsHighlighter::QmlProperty] = createFormat("#9999aa");
    m_formats[QQmlJsHighlighter::QmlType]     = createFormat("#aaa");
    m_formats[QQmlJsHighlighter::QmlRuntimeBoundProperty] = createFormat("#333");
    m_formats[QQmlJsHighlighter::QmlRuntimeModifiedValue] = createFormat("#333");
    m_formats[QQmlJsHighlighter::QmlEdit] = createFormat("#fff", "#0b273f");


    // built-in and other popular objects + properties
    m_knownIds << "Object";
    m_knownIds << "prototype";
    m_knownIds << "property";
    m_knownIds << "__parent__";
    m_knownIds << "__proto__";
    m_knownIds << "__defineGetter__";
    m_knownIds << "__defineSetter__";
    m_knownIds << "__lookupGetter__";
    m_knownIds << "__lookupSetter__";
    m_knownIds << "__noSuchMethod__";
    m_knownIds << "Function";
    m_knownIds << "String";
    m_knownIds << "Array";
    m_knownIds << "RegExp";
    m_knownIds << "global";
    m_knownIds << "NaN";
    m_knownIds << "undefined";
    m_knownIds << "Math";
    m_knownIds << "import";
    m_knownIds << "string";
    m_knownIds << "int";
    m_knownIds << "variant";
    m_knownIds << "signal";
}

// Walks through ab.bc.cd to find whether it preceedes a colon or an opening brace
// depending on the case, this can be an qml type or a qml property
// Anything else is discarded
QQmlJsHighlighter::LookAheadType QQmlJsHighlighter::lookAhead(
        const QString& text,
        const QList<QmlJS::Token> &tokens,
        QList<QmlJS::Token>::ConstIterator it,
        int state)
{
    bool identifierExpected = false;
    while ( it != tokens.end() ){
        const QmlJS::Token& tk = *it;
        if ( tk.is(QmlJS::Token::Colon) ){
            return QQmlJsHighlighter::Property;
        } else if ( tk.is(QmlJS::Token::LeftBrace) ){
            return QQmlJsHighlighter::Type;
        } else if ( tk.is(QmlJS::Token::Identifier ) ){
            if ( !identifierExpected ){
                if ( tk.length == 2 && text.mid(tk.begin(), tk.length) == "on" )
                     return QQmlJsHighlighter::Type;

                return QQmlJsHighlighter::Unknown;
            }
            identifierExpected = false;
        } else if ( tk.is(QmlJS::Token::Dot) ){
            identifierExpected = true;
        } else if ( tk.isNot(QmlJS::Token::Comment) ){
            return QQmlJsHighlighter::Unknown;
        }
        ++it;
    }

    QTextBlock bl = currentBlock().next();
    while ( bl.isValid() ){
        QmlJS::Scanner scn;
        QList<QmlJS::Token> tks = scn(bl.text(), state);
        state = scn.state();
        it = tks.begin();

        while ( it != tks.end() ){
            const QmlJS::Token& tk = *it;
            if ( tk.is(QmlJS::Token::Colon) ){
                return QQmlJsHighlighter::Property;
            } else if ( tk.is(QmlJS::Token::LeftBrace) ){
                return QQmlJsHighlighter::Type;
            } else if ( tk.is(QmlJS::Token::Identifier ) ){
                if ( !identifierExpected ){
                    if ( tk.length == 2 && text.mid(tk.begin(), tk.length) == "on" )
                         return QQmlJsHighlighter::Type;

                    return QQmlJsHighlighter::Unknown;
                }
                identifierExpected = false;
            } else if ( tk.is(QmlJS::Token::Dot) ){
                identifierExpected = true;
            } else if ( tk.isNot(QmlJS::Token::Comment) ){
                return QQmlJsHighlighter::Unknown;
            }
            ++it;
        }

        bl = bl.next();
    }

    return QQmlJsHighlighter::Unknown;
}

void QQmlJsHighlighter::highlightBlock(const QString &text){

    QList<int> bracketPositions;
    int blockState   = previousBlockState();

    bool prevGenerated   = (blockState >> 4) & 1;
    int bracketLevel = blockState >> 5;
    int state        = blockState & 15;


    if (blockState < 0) {
        prevGenerated = false;
        bracketLevel = 0;
        state = QmlJS::Scanner::Normal;
    }

    QmlJS::Scanner scanner;
    QList<QmlJS::Token> tokens = scanner(text, state);
    state = scanner.state();

    QList<QmlJS::Token>::iterator it = tokens.begin();
    while ( it != tokens.end() ){
        QmlJS::Token& tk = *it;
        switch(tk.kind){
        case QmlJS::Token::Keyword:
            setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::Keyword]);
            break;
        case QmlJS::Token::Identifier:{
            QString tktext = text.mid(tk.begin(), tk.length);
            if ( m_knownIds.contains(tktext) ){
                setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::Identifier]);
            } else if ( tktext == "true" || tktext == "false" ){
                setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::Keyword]);
            } else {
                QList<QmlJS::Token>::iterator lait = it;
                QQmlJsHighlighter::LookAheadType la = lookAhead(text, tokens, ++lait, state);
                if ( la == QQmlJsHighlighter::Property )
                    setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::QmlProperty]);
                else if ( la == QQmlJsHighlighter::Type )
                    setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::QmlType]);
            }
            break;
        }
        case QmlJS::Token::String:
            setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::String]);
            break;
        case QmlJS::Token::Comment:
            setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::Comment]);
            break;
        case QmlJS::Token::Number:
            setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::Number]);
            break;
        case QmlJS::Token::LeftParenthesis:
        case QmlJS::Token::RightParenthesis:
        case QmlJS::Token::LeftBrace:
        case QmlJS::Token::RightBrace:
        case QmlJS::Token::LeftBracket:
        case QmlJS::Token::RightBracket:
            break;
        case QmlJS::Token::Semicolon:
        case QmlJS::Token::Colon:
        case QmlJS::Token::Comma:
        case QmlJS::Token::Dot:
        case QmlJS::Token::Delimiter:
            setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::Operator]);
            break;
        case QmlJS::Token::RegExp:
            setFormat(tk.begin(), tk.length, m_formats[QQmlJsHighlighter::String]);
            break;
        }

        ++it;
    }

    lcv::QProjectDocumentBlockData *blockData =
            reinterpret_cast<lcv::QProjectDocumentBlockData*>(currentBlock().userData());

    if (!bracketPositions.isEmpty()) {
        if (!blockData) {
            blockData = new lcv::QProjectDocumentBlockData;
            currentBlock().setUserData(blockData);
        }
        blockData->bracketPositions = bracketPositions;
    }

    bool generated = false;

    if ( prevGenerated ){
        QTextBlock prevBlock = currentBlock().previous();
        if ( prevBlock.isValid() && prevBlock.userData() ){
            lcv::QProjectDocumentBlockData *prevBlockData =
                    reinterpret_cast<lcv::QProjectDocumentBlockData*>(prevBlock.userData());

            if ( prevBlockData->exceededBindingLength > 0 ){
                int currentExceededLength = prevBlockData->exceededBindingLength - currentBlock().length();
                if ( currentExceededLength > 0 ){
                    setFormat(0, currentBlock().length(), QColor("#aa00aa") );

                    if (!blockData) {
                        blockData = new lcv::QProjectDocumentBlockData;
                        currentBlock().setUserData(blockData);
                    }
                    blockData->exceededBindingLength = currentExceededLength;
                    generated = true;
                } else {
                    setFormat(0, prevBlockData->exceededBindingLength, QColor("#aa00aa") );
                }
            }
        }
    }


    if ( blockData ){
        foreach(lcv::QProjectDocumentBinding* bind, blockData->m_bindings ){
            setFormat(bind->propertyPosition - currentBlock().position(), bind->propertyLength, QColor("#ff0000"));

            if ( bind->modifiedByEngine ){
                int valueFrom = bind->propertyPosition + bind->propertyLength + bind->valuePositionOffset;
                setFormat(valueFrom - currentBlock().position(), bind->valueLength, QColor("#ff00ff"));
                if ( valueFrom + bind->valueLength > currentBlock().position() + currentBlock().length() ){
                    generated = true;
                    blockData->exceededBindingLength =
                            bind->valueLength - (currentBlock().length() - (valueFrom - currentBlock().position()));
                }
            }
        }
    }

    blockState = (state & 15) | (generated << 4) | (bracketLevel << 5);
    setCurrentBlockState(blockState);

    if ( m_documentState && m_documentState->editingFragment() ){
        int position = m_documentState->editingFragment()->position();
        int length   = m_documentState->editingFragment()->length();
        if ( position + length >= currentBlock().position() &&
             position < currentBlock().position() + currentBlock().length())
        {
            int from = position - currentBlock().position();
            setFormat(from < 0 ? 0 : from, length, m_formats[QQmlJsHighlighter::QmlEdit]);
        }
    }
}





}// namespace
