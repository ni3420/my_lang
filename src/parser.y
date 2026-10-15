%code requires {
#include "myl/ast.hpp"
#include <memory>
#include <string>
#include <vector>

extern const char* g_myl_filename;
}

%code {
#include <cstdio>
#include <cstdlib>

// Global sink for parse diagnostics, set by the frontend.
std::vector<std::string>* g_parse_errors = nullptr;

int yylex(void);
void yyerror(myl::Program*& result, const char* msg);

static myl::Location loc_of(const YYLTYPE& l) {
    return myl::Location(l.first_line, l.first_column, g_myl_filename);
}

static myl::Type to_type(int kind) {
    return myl::Type(static_cast<myl::TypeKind>(kind));
}

static std::vector<myl::StmtPtr> take_stmts(std::vector<myl::Stmt*>* v) {
    std::vector<myl::StmtPtr> out;
    out.reserve(v->size());
    for (auto* s : *v) out.emplace_back(s);
    delete v;
    return out;
}

static std::vector<myl::ExprPtr> take_exprs(std::vector<myl::Expr*>* v) {
    std::vector<myl::ExprPtr> out;
    out.reserve(v->size());
    for (auto* e : *v) out.emplace_back(e);
    delete v;
    return out;
}

static std::vector<myl::DeclPtr> take_decls(std::vector<myl::Decl*>* v) {
    std::vector<myl::DeclPtr> out;
    out.reserve(v->size());
    for (auto* d : *v) out.emplace_back(d);
    delete v;
    return out;
}

static myl::ExprPtr wrap(myl::Expr* e) { return myl::ExprPtr(e); }
static myl::StmtPtr wrap_stmt(myl::Stmt* s) { return myl::StmtPtr(s); }
}

%locations
%define parse.error verbose
%parse-param { myl::Program*& result }

%union {
    std::string* str;
    long long intval;
    double floatval;
    myl::Expr* expr;
    myl::Stmt* stmt;
    myl::Decl* decl;
    myl::Program* prog;
    std::vector<myl::Stmt*>* stmt_vec;
    std::vector<myl::Decl*>* decl_vec;
    std::vector<myl::Expr*>* expr_vec;
    std::vector<myl::FunctionDecl::Param>* param_vec;
    myl::FunctionDecl::Param* param;
    int typekind;
}

%token <str> Identifier IntegerLiteral FloatLiteral StringLiteral
%token TrueKw FalseKw SahiKw GalatKw
%token LetKw MutKw FnKw ReturnKw IfKw ElseKw WhileKw PrintKw
%token RakhoKw BoloKw AgarKw WarnaKw RepeatKw KaamKw BhejoKw AnkKw
%token IntKw FloatKw BoolKw StringKw VoidKw
%token And Or Not
%token Plus Minus Star Slash Percent
%token Eq Ne Lt Le Gt Ge
%token Arrow
%token Assign Semicolon Colon Comma
%token LParen RParen LBrace RBrace

%type <prog> program
%type <decl_vec> declaration_list
%type <decl> declaration var_decl function_decl
%type <typekind> type_spec
%type <stmt_vec> block statement_list
%type <stmt> statement if_stmt while_stmt return_stmt expr_stmt
%type <stmt> assignment_stmt var_decl_stmt bolo_stmt
%type <expr> expression logical_or logical_and equality comparison term factor unary primary
%type <expr_vec> argument_list
%type <param_vec> parameter_list
%type <param> parameter

%precedence LOWEST
%precedence Or
%precedence And
%precedence Eq Ne
%precedence Lt Le Gt Ge
%precedence Plus Minus
%precedence Star Slash Percent
%precedence UNARY

%start program

%%

program:
    declaration_list {
        std::vector<myl::DeclPtr> decls = take_decls($1);
        std::vector<myl::DeclPtr> fnDecls;
        std::vector<myl::StmtPtr> mainStmts;
        myl::Location mainLoc;

        for (auto& d : decls) {
            if (auto* sd = dynamic_cast<myl::StmtDecl*>(d.get())) {
                if (mainStmts.empty()) mainLoc = sd->loc;
                mainStmts.push_back(std::move(sd->stmt));
            } else {
                fnDecls.push_back(std::move(d));
            }
        }

        result = new myl::Program();

        // Script mode: top-level statements are wrapped into an implicit
        // `main`. Programs that only contain function declarations are
        // emitted unchanged.
        if (!mainStmts.empty()) {
            mainLoc.filename = g_myl_filename;
            mainStmts.push_back(myl::StmtPtr(new myl::ReturnStmt(mainLoc,
                myl::ExprPtr(new myl::IntegerLiteral(mainLoc, 0)))));
            fnDecls.push_back(myl::DeclPtr(new myl::FunctionDecl(mainLoc, "main",
                std::vector<myl::FunctionDecl::Param>(), myl::Type::intType(),
                std::move(mainStmts))));
        }

        result->declarations = std::move(fnDecls);
    }
;

declaration_list:
    %empty { $$ = new std::vector<myl::Decl*>(); }
    | declaration_list declaration { $1->push_back($2); $$ = $1; }
;

declaration:
    function_decl { $$ = $1; }
    | statement { $$ = new myl::StmtDecl(loc_of(@1), wrap_stmt($1)); }
;

var_decl:
    LetKw MutKw Identifier Colon type_spec Assign expression Semicolon {
        $$ = new myl::VarDecl(loc_of(@1), *$3, to_type($5), wrap($7), true);
        delete $3;
    }
    | LetKw Identifier Colon type_spec Assign expression Semicolon {
        $$ = new myl::VarDecl(loc_of(@1), *$2, to_type($4), wrap($6), false);
        delete $2;
    }
    | LetKw MutKw Identifier Assign expression Semicolon {
        $$ = new myl::VarDecl(loc_of(@1), *$3, myl::Type(), wrap($5), true);
        delete $3;
    }
    | LetKw Identifier Assign expression Semicolon {
        $$ = new myl::VarDecl(loc_of(@1), *$2, myl::Type(), wrap($4), false);
        delete $2;
    }
    | RakhoKw MutKw Identifier Colon type_spec Assign expression Semicolon {
        $$ = new myl::VarDecl(loc_of(@1), *$3, to_type($5), wrap($7), true);
        delete $3;
    }
    | RakhoKw Identifier Colon type_spec Assign expression Semicolon {
        $$ = new myl::VarDecl(loc_of(@1), *$2, to_type($4), wrap($6), true);
        delete $2;
    }
    | RakhoKw MutKw Identifier Assign expression Semicolon {
        $$ = new myl::VarDecl(loc_of(@1), *$3, myl::Type(), wrap($5), true);
        delete $3;
    }
    | RakhoKw Identifier Assign expression Semicolon {
        $$ = new myl::VarDecl(loc_of(@1), *$2, myl::Type(), wrap($4), true);
        delete $2;
    }
;

function_decl:
    FnKw Identifier LParen parameter_list RParen Arrow type_spec block {
        $$ = new myl::FunctionDecl(loc_of(@1), *$2, std::move(*$4), to_type($7), take_stmts($8));
        delete $2; delete $4;
    }
    | FnKw Identifier LParen parameter_list RParen block {
        $$ = new myl::FunctionDecl(loc_of(@1), *$2, std::move(*$4),
                                   myl::Type::voidType(), take_stmts($6));
        delete $2; delete $4;
    }
    | FnKw Identifier LParen RParen Arrow type_spec block {
        $$ = new myl::FunctionDecl(loc_of(@1), *$2,
                                   std::vector<myl::FunctionDecl::Param>(),
                                   to_type($6), take_stmts($7));
        delete $2;
    }
    | FnKw Identifier LParen RParen block {
        $$ = new myl::FunctionDecl(loc_of(@1), *$2,
                                   std::vector<myl::FunctionDecl::Param>(),
                                   myl::Type::voidType(), take_stmts($5));
        delete $2;
    }
    | KaamKw Identifier LParen parameter_list RParen Arrow type_spec block {
        $$ = new myl::FunctionDecl(loc_of(@1), *$2, std::move(*$4), to_type($7), take_stmts($8));
        delete $2; delete $4;
    }
    | KaamKw Identifier LParen parameter_list RParen block {
        $$ = new myl::FunctionDecl(loc_of(@1), *$2, std::move(*$4),
                                   myl::Type::voidType(), take_stmts($6));
        delete $2; delete $4;
    }
    | KaamKw Identifier LParen RParen Arrow type_spec block {
        $$ = new myl::FunctionDecl(loc_of(@1), *$2,
                                   std::vector<myl::FunctionDecl::Param>(),
                                   to_type($6), take_stmts($7));
        delete $2;
    }
    | KaamKw Identifier LParen RParen block {
        $$ = new myl::FunctionDecl(loc_of(@1), *$2,
                                   std::vector<myl::FunctionDecl::Param>(),
                                   myl::Type::voidType(), take_stmts($5));
        delete $2;
    }
;

type_spec:
    AnkKw    { $$ = static_cast<int>(myl::TypeKind::Int); }
    | IntKw    { $$ = static_cast<int>(myl::TypeKind::Int); }
    | FloatKw  { $$ = static_cast<int>(myl::TypeKind::Float); }
    | BoolKw   { $$ = static_cast<int>(myl::TypeKind::Bool); }
    | StringKw { $$ = static_cast<int>(myl::TypeKind::String); }
    | VoidKw   { $$ = static_cast<int>(myl::TypeKind::Void); }
;

parameter_list:
    %empty { $$ = new std::vector<myl::FunctionDecl::Param>(); }
    | parameter_list Comma parameter { $1->push_back(*$3); delete $3; $$ = $1; }
    | parameter { $$ = new std::vector<myl::FunctionDecl::Param>(); $$->push_back(*$1); delete $1; }
;

parameter:
    Identifier Colon type_spec {
        $$ = new myl::FunctionDecl::Param{ *$1, to_type($3) };
        delete $1;
    }
;

block:
    LBrace statement_list RBrace { $$ = $2; }
;

statement_list:
    %empty { $$ = new std::vector<myl::Stmt*>(); }
    | statement_list statement { $1->push_back($2); $$ = $1; }
;

statement:
    if_stmt { $$ = $1; }
    | while_stmt { $$ = $1; }
    | return_stmt { $$ = $1; }
    | assignment_stmt { $$ = $1; }
    | var_decl_stmt { $$ = $1; }
    | bolo_stmt { $$ = $1; }
    | expr_stmt { $$ = $1; }
    | block { $$ = new myl::BlockStmt(loc_of(@1), take_stmts($1)); }
;

bolo_stmt:
    BoloKw expression Semicolon {
        std::vector<myl::ExprPtr> args;
        args.push_back(wrap($2));
        $$ = new myl::ExprStmt(loc_of(@1),
                myl::ExprPtr(new myl::CallExpr(loc_of(@1), "print", std::move(args))));
    }
;

var_decl_stmt:
    var_decl {
        $$ = new myl::VarDeclStmt(loc_of(@1), static_cast<myl::VarDecl*>($1));
    }
;

assignment_stmt:
    Identifier Assign expression Semicolon {
        $$ = new myl::AssignStmt(loc_of(@1), *$1, wrap($3));
        delete $1;
    }
;

if_stmt:
    IfKw LParen expression RParen statement ElseKw statement {
        std::vector<myl::StmtPtr> thenB; thenB.push_back(wrap_stmt($5));
        std::vector<myl::StmtPtr> elseB; elseB.push_back(wrap_stmt($7));
        $$ = new myl::IfStmt(loc_of(@1), wrap($3), std::move(thenB), std::move(elseB));
    }
    | IfKw LParen expression RParen statement {
        std::vector<myl::StmtPtr> thenB; thenB.push_back(wrap_stmt($5));
        $$ = new myl::IfStmt(loc_of(@1), wrap($3), std::move(thenB),
                             std::vector<myl::StmtPtr>());
    }
    | AgarKw LParen expression RParen statement WarnaKw statement {
        std::vector<myl::StmtPtr> thenB; thenB.push_back(wrap_stmt($5));
        std::vector<myl::StmtPtr> elseB; elseB.push_back(wrap_stmt($7));
        $$ = new myl::IfStmt(loc_of(@1), wrap($3), std::move(thenB), std::move(elseB));
    }
    | AgarKw LParen expression RParen statement {
        std::vector<myl::StmtPtr> thenB; thenB.push_back(wrap_stmt($5));
        $$ = new myl::IfStmt(loc_of(@1), wrap($3), std::move(thenB),
                             std::vector<myl::StmtPtr>());
    }
;

while_stmt:
    WhileKw LParen expression RParen statement {
        std::vector<myl::StmtPtr> body; body.push_back(wrap_stmt($5));
        $$ = new myl::WhileStmt(loc_of(@1), wrap($3), std::move(body));
    }
    | RepeatKw LParen expression RParen statement {
        std::vector<myl::StmtPtr> body; body.push_back(wrap_stmt($5));
        $$ = new myl::WhileStmt(loc_of(@1), wrap($3), std::move(body));
    }
;

return_stmt:
    ReturnKw expression Semicolon { $$ = new myl::ReturnStmt(loc_of(@1), wrap($2)); }
    | ReturnKw Semicolon { $$ = new myl::ReturnStmt(loc_of(@1), myl::ExprPtr()); }
    | BhejoKw expression Semicolon { $$ = new myl::ReturnStmt(loc_of(@1), wrap($2)); }
    | BhejoKw Semicolon { $$ = new myl::ReturnStmt(loc_of(@1), myl::ExprPtr()); }
;

expr_stmt:
    expression Semicolon { $$ = new myl::ExprStmt(loc_of(@1), wrap($1)); }
;

expression:
    logical_or { $$ = $1; }
;

logical_or:
    logical_and { $$ = $1; }
    | logical_or Or logical_and {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Or, wrap($1), wrap($3));
    }
;

logical_and:
    equality { $$ = $1; }
    | logical_and And equality {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::And, wrap($1), wrap($3));
    }
;

equality:
    comparison { $$ = $1; }
    | equality Eq comparison {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Eq, wrap($1), wrap($3));
    }
    | equality Ne comparison {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Ne, wrap($1), wrap($3));
    }
;

comparison:
    term { $$ = $1; }
    | comparison Lt term {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Lt, wrap($1), wrap($3));
    }
    | comparison Le term {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Le, wrap($1), wrap($3));
    }
    | comparison Gt term {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Gt, wrap($1), wrap($3));
    }
    | comparison Ge term {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Ge, wrap($1), wrap($3));
    }
;

term:
    factor { $$ = $1; }
    | term Plus factor {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Add, wrap($1), wrap($3));
    }
    | term Minus factor {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Sub, wrap($1), wrap($3));
    }
;

factor:
    unary { $$ = $1; }
    | factor Star unary {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Mul, wrap($1), wrap($3));
    }
    | factor Slash unary {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Div, wrap($1), wrap($3));
    }
    | factor Percent unary {
        $$ = new myl::BinaryExpr(loc_of(@2), myl::BinaryExpr::Op::Mod, wrap($1), wrap($3));
    }
;

unary:
    primary { $$ = $1; }
    | Minus unary %prec UNARY {
        $$ = new myl::UnaryExpr(loc_of(@1), myl::UnaryExpr::Op::Neg, wrap($2));
    }
    | Not unary %prec UNARY {
        $$ = new myl::UnaryExpr(loc_of(@1), myl::UnaryExpr::Op::Not, wrap($2));
    }
;

primary:
    IntegerLiteral { $$ = new myl::IntegerLiteral(loc_of(@1), std::stoll(*$1)); delete $1; }
    | FloatLiteral { $$ = new myl::FloatLiteral(loc_of(@1), std::stod(*$1)); delete $1; }
    | TrueKw { $$ = new myl::BoolLiteral(loc_of(@1), true); }
    | FalseKw { $$ = new myl::BoolLiteral(loc_of(@1), false); }
    | SahiKw { $$ = new myl::BoolLiteral(loc_of(@1), true); }
    | GalatKw { $$ = new myl::BoolLiteral(loc_of(@1), false); }
    | StringLiteral { $$ = new myl::StringLiteral(loc_of(@1), *$1); delete $1; }
    | Identifier { $$ = new myl::Identifier(loc_of(@1), *$1); delete $1; }
    | PrintKw LParen argument_list RParen {
        $$ = new myl::CallExpr(loc_of(@1), "print", take_exprs($3));
    }
    | Identifier LParen argument_list RParen {
        $$ = new myl::CallExpr(loc_of(@1), *$1, take_exprs($3));
        delete $1;
    }
    | LParen expression RParen { $$ = $2; }
;

argument_list:
    %empty { $$ = new std::vector<myl::Expr*>(); }
    | argument_list Comma expression { $1->push_back($3); $$ = $1; }
    | expression { $$ = new std::vector<myl::Expr*>(); $$->push_back($1); }
;

%%

void yyerror(myl::Program*& result, const char* msg) {
    (void)result;
    if (!g_parse_errors) return;
    std::string where = g_myl_filename ? g_myl_filename : "<unknown>";
    g_parse_errors->push_back(where + ":" + std::to_string(yylloc.first_line) + ":" +
                              std::to_string(yylloc.first_column) + ": " + msg);
}
