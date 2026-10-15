%option noyywrap yylineno never-interactive

%{
#include "myl/frontend.hpp"
#include "parser.hpp"

#include <cstring>
#include <string>
#include <vector>

// yylval / yylloc are declared by the Bison-generated parser.hpp above.

// Current filename, used to build source locations.
const char* g_myl_filename = "<unknown>";

// Captures the text of the token currently being matched (for the tokenize API).
static std::string g_last_text;

// Manual column tracking (flex provides yylineno).
int yycolumn = 1;

// Buffer used while scanning a string literal.
static std::string string_buf;

#define YY_USER_ACTION                                            \
    do {                                                          \
        g_last_text.assign(yytext, (size_t)yyleng);               \
        yylloc.first_line = yylineno;                             \
        yylloc.first_column = yycolumn;                           \
        yylloc.last_line = yylineno;                              \
        yylloc.last_column = yycolumn + (int)yyleng - 1;          \
        yycolumn += (int)yyleng;                                  \
    } while (0);

static void count_newlines(const char* s, int len) {
    for (int i = 0; i < len; ++i) {
        if (s[i] == '\n') yycolumn = 1;
    }
}
%}

DIGIT      [0-9]
IDENT      [a-zA-Z_][a-zA-Z0-9_]*

%x STRING

%%

"//"[^\n]*           { /* line comment */ }
"/*"([^*]|\*+[^*/])*\*+"/" { /* block comment */ }

"let"        { return LetKw; }
"mut"        { return MutKw; }
"fn"         { return FnKw; }
"return"     { return ReturnKw; }
"if"         { return IfKw; }
"else"       { return ElseKw; }
"while"      { return WhileKw; }
"print"      { return PrintKw; }
"int"        { return IntKw; }
"float"      { return FloatKw; }
"bool"       { return BoolKw; }
"string"     { return StringKw; }
"void"       { return VoidKw; }
"true"       { return TrueKw; }
"false"      { return FalseKw; }
"and"        { return And; }
"or"         { return Or; }
"not"        { return Not; }

"rakho"      { return RakhoKw; }
"bolo"       { return BoloKw; }
"agar"       { return AgarKw; }
"warna"      { return WarnaKw; }
"repeat"     { return RepeatKw; }
"kaam"       { return KaamKw; }
"bhejo"      { return BhejoKw; }
"sahi"       { return SahiKw; }
"galat"      { return GalatKw; }
"ank"        { return AnkKw; }

{IDENT}      { yylval.str = new std::string(yytext); return Identifier; }

{DIGIT}+                  { yylval.str = new std::string(yytext); return IntegerLiteral; }
{DIGIT}+"."{DIGIT}*       { yylval.str = new std::string(yytext); return FloatLiteral; }
{DIGIT}*"."{DIGIT}+       { yylval.str = new std::string(yytext); return FloatLiteral; }

\"           { string_buf.clear(); BEGIN(STRING); }

<STRING>{
    \\\"       { string_buf += '"'; }
    \\\\       { string_buf += '\\'; }
    \\n        { string_buf += '\n'; }
    \\t        { string_buf += '\t'; }
    \\r        { string_buf += '\r'; }
    \\.        { string_buf += yytext[1]; }
    [^\"\\\n]+ { string_buf += yytext; }
    \"         { yylval.str = new std::string(string_buf); BEGIN(INITIAL); return StringLiteral; }
    \n         { /* newline inside string is allowed */ }
    <<EOF>>    { BEGIN(INITIAL); return 0; }
}

"->"         { return Arrow; }
"=="         { return Eq; }
"!="         { return Ne; }
"<="         { return Le; }
">="         { return Ge; }
"<"          { return Lt; }
">"          { return Gt; }
"="          { return Assign; }
"+"          { return Plus; }
"-"          { return Minus; }
"*"          { return Star; }
"/"          { return Slash; }
"%"          { return Percent; }
";"          { return Semicolon; }
":"          { return Colon; }
","          { return Comma; }
"("          { return LParen; }
")"          { return RParen; }
"{"          { return LBrace; }
"}"          { return RBrace; }

[ \t\r]+     { }
\n           { count_newlines(yytext, yyleng); }

.            { return yytext[0]; }

<<EOF>>      { return 0; }

%%

static YY_BUFFER_STATE g_current_buffer = nullptr;

// Namespace-scope helpers used by the compiler driver.
extern "C" void myl_set_input(const char* source, const char* filename) {
    if (g_current_buffer) {
        yy_delete_buffer(g_current_buffer);
        g_current_buffer = nullptr;
    }
    g_current_buffer = yy_scan_string(source);
    g_myl_filename = filename;
    yylineno = 1;
    yycolumn = 1;
}

extern "C" void myl_cleanup_input() {
    if (g_current_buffer) {
        yy_delete_buffer(g_current_buffer);
        g_current_buffer = nullptr;
    }
}

namespace myl {

static const char* token_kind_name(int tok) {
    switch (tok) {
        case LetKw: return "let";
        case MutKw: return "mut";
        case FnKw: return "fn";
        case ReturnKw: return "return";
        case IfKw: return "if";
        case ElseKw: return "else";
        case WhileKw: return "while";
        case PrintKw: return "print";
        case IntKw: return "int";
        case FloatKw: return "float";
        case BoolKw: return "bool";
        case StringKw: return "string";
        case VoidKw: return "void";
        case TrueKw: return "true";
        case FalseKw: return "false";
        case And: return "and";
        case Or: return "or";
        case Not: return "not";
        case RakhoKw: return "rakho";
        case BoloKw: return "bolo";
        case AgarKw: return "agar";
        case WarnaKw: return "warna";
        case RepeatKw: return "repeat";
        case KaamKw: return "kaam";
        case BhejoKw: return "bhejo";
        case SahiKw: return "sahi";
        case GalatKw: return "galat";
        case AnkKw: return "ank";
        case ::Identifier: return "identifier";
        case ::IntegerLiteral: return "integer";
        case ::FloatLiteral: return "float_literal";
        case ::StringLiteral: return "string_literal";
        case Arrow: return "->";
        case Eq: return "==";
        case Ne: return "!=";
        case Le: return "<=";
        case Ge: return ">=";
        case Lt: return "<";
        case Gt: return ">";
        case Assign: return "=";
        case Plus: return "+";
        case Minus: return "-";
        case Star: return "*";
        case Slash: return "/";
        case Percent: return "%";
        case Semicolon: return ";";
        case Colon: return ":";
        case Comma: return ",";
        case LParen: return "(";
        case RParen: return ")";
        case LBrace: return "{";
        case RBrace: return "}";
        default: return "unknown";
    }
}

std::vector<TokenInfo> tokenize(const std::string& source,
                                const std::string& filename,
                                std::vector<std::string>& errors) {
    (void)errors;
    std::vector<TokenInfo> result;
    myl_set_input(source.c_str(), filename.c_str());

    int line = 1;
    int column = 1;
    int tok;
    while ((tok = yylex()) != 0) {
        TokenInfo info;
        info.kind = token_kind_name(tok);
        info.text = g_last_text;
        info.line = line;
        info.column = column;

        // Track positions independently of the parser's yylloc bookkeeping.
        for (char c : g_last_text) {
            if (c == '\n') { line++; column = 1; } else { column++; }
        }

        result.push_back(std::move(info));
    }

    myl_cleanup_input();
    return result;
}

} // namespace myl
