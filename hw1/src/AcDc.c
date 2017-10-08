#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "header.h"


int main( int argc, char *argv[] )
{
	FILE *source, *target;
	Program program;
	SymbolTable symtab;

	if( argc == 3){
		source = fopen(argv[1], "r");
		target = fopen(argv[2], "w");
		if( !source ){
			printf("can't open the source file\n");
			exit(2);
		}
		else if( !target ){
			printf("can't open the target file\n");
			exit(2);
		}
		else{
			program = parser(source);
			fclose(source);
			symtab = build(program);
			check(&program, &symtab);
			folding(program.statements);
			gencode(program, target, &symtab);
		}
	}
	else{
		printf("Usage: %s source_file target_file\n", argv[0]);
	}


	return 0;
}
/* Constant folding */
void folding(Statements *stmts)
{
	if (stmts->first.type == Assignment) {
		//print_expr(stmts->first.stmt.assign.expr);puts("\n");
		stmts->first.stmt.assign.expr = exprFolding(stmts->first.stmt.assign.expr);
	}
	if (stmts->rest != NULL)
		folding(stmts->rest);
}

Expression *exprFolding(Expression *exprnode) {
	Expression *left = exprnode->leftOperand;
	Expression *right = exprnode->rightOperand;
	
	if (left != NULL && left->v.type == IntToFloatConvertNode)
		left = left->leftOperand;
	if (right != NULL && right->v.type == IntToFloatConvertNode)
		right = right->leftOperand;
	
	if (left != NULL)
		left = exprFolding(left);
	if (right != NULL)
		right = exprFolding(right);
	
	if (left == NULL || right == NULL)
		return exprnode;
	
	if(left->v.type == IntConst && right->v.type == IntConst) {
		switch (exprnode->v.type) {
			case PlusNode:
				exprnode->v.val.ivalue = left->v.val.ivalue + right->v.val.ivalue;
				break;
			case MinusNode:
				exprnode->v.val.ivalue = left->v.val.ivalue - right->v.val.ivalue;
				break;
			case MulNode:
				exprnode->v.val.ivalue = left->v.val.ivalue * right->v.val.ivalue;
				break;
			case DivNode:
				exprnode->v.val.ivalue = left->v.val.ivalue / right->v.val.ivalue;
				break;
			default:
				break;
		}
		exprnode->v.type = IntConst;
		free(left);
		free(right);
		exprnode->leftOperand = exprnode->rightOperand = NULL;
	} else if (left->v.type == FloatConst && right->v.type == FloatConst) {
		switch (exprnode->v.type) {
			case PlusNode:
				exprnode->v.val.fvalue = left->v.val.fvalue + right->v.val.fvalue;
				break;
			case MinusNode:
				exprnode->v.val.fvalue = left->v.val.fvalue - right->v.val.fvalue;
				break;
			case MulNode:
				exprnode->v.val.fvalue = left->v.val.fvalue * right->v.val.fvalue;
				break;
			case DivNode:
				exprnode->v.val.fvalue = left->v.val.fvalue / right->v.val.fvalue;
				break;
			default:
				break;
		}
		exprnode->v.type = FloatConst;
		free(left);
		free(right);
		exprnode->leftOperand = exprnode->rightOperand = NULL;
	} else if (left->v.type == IntConst && right->v.type == FloatConst) {
		switch (exprnode->v.type) {
			case PlusNode:
				exprnode->v.val.fvalue = left->v.val.ivalue + right->v.val.fvalue;
				break;
			case MinusNode:
				exprnode->v.val.fvalue = left->v.val.ivalue - right->v.val.fvalue;
				break;
			case MulNode:
				exprnode->v.val.fvalue = left->v.val.ivalue * right->v.val.fvalue;
				break;
			case DivNode:
				exprnode->v.val.fvalue = left->v.val.ivalue / right->v.val.fvalue;
				break;
			default:
				break;
		}
		exprnode->v.type = FloatConst;
		free(left);
		free(right);
		exprnode->leftOperand = exprnode->rightOperand = NULL;
	} else if (left->v.type == FloatConst && right->v.type == IntConst) {
		switch (exprnode->v.type) {
			case PlusNode:
				exprnode->v.val.fvalue = left->v.val.fvalue + right->v.val.ivalue;
				break;
			case MinusNode:
				exprnode->v.val.fvalue = left->v.val.fvalue - right->v.val.ivalue;
				break;
			case MulNode:
				exprnode->v.val.fvalue = left->v.val.fvalue * right->v.val.ivalue;
				break;
			case DivNode:
				exprnode->v.val.fvalue = left->v.val.fvalue / right->v.val.ivalue;
				break;
			default:
				break;
		}
		exprnode->v.type = FloatConst;
		free(left);
		free(right);
		exprnode->leftOperand = exprnode->rightOperand = NULL;
	}


	return exprnode;
}
 
 
/********************************************* 
  Scanning 
 *********************************************/
Token getNumericToken( FILE *source, char c )
{
	Token token;
	int i = 0;

	while( isdigit(c) ) {
		token.tok[i++] = c;
		c = fgetc(source);
	}

	if( c != '.' ){
		ungetc(c, source);
		token.tok[i] = '\0';
		token.type = IntValue;
		return token;
	}

	token.tok[i++] = '.';

	c = fgetc(source);
	if( !isdigit(c) ){
		ungetc(c, source);
		printf("Expect a digit : %c\n", c);
		exit(1);
	}

	while( isdigit(c) ){
		token.tok[i++] = c;
		c = fgetc(source);
	}

	ungetc(c, source);
	token.tok[i] = '\0';
	token.type = FloatValue;
	return token;
}

Token scanner( FILE *source )
{
	char c;
	Token token;

	while( !feof(source) ){
		c = fgetc(source);

		while( isspace(c) ) c = fgetc(source);

		if( isdigit(c) )
			return getNumericToken(source, c);

		token.tok[0] = c;
		token.tok[1] = '\0';
		if( islower(c) ){
			if( c == 'f' )
				token.type = FloatDeclaration;
			else if( c == 'i' )
				token.type = IntegerDeclaration;
			else if( c == 'p' )
				token.type = PrintOp;
			else {
				int i = 1;
				c = fgetc(source);
				while (islower(c)) {
					token.tok[i++] = c;
					c = fgetc(source);
				}
				token.tok[i] = '\0';
				token.type = Alphabet;
				ungetc(c, source);
			}
			return token;
		}

		switch(c){
			case '=':
				token.type = AssignmentOp;
				return token;
			case '+':
				token.type = PlusOp;
				return token;
			case '-':
				token.type = MinusOp;
				return token;
			case '*':
				token.type = MulOp;
				return token;
			case '/':
				token.type = DivOp;
				return token;
			case EOF:
				token.type = EOFsymbol;
				token.tok[0] = '\0';
				return token;
			default:
				printf("Invalid character : %c\n", c);
				exit(1);
		}
	}

	token.tok[0] = '\0';
	token.type = EOFsymbol;
	return token;
}


/********************************************************
  Parsing
 *********************************************************/
Declaration parseDeclaration( FILE *source, Token token )
{
	Token token2;
	switch(token.type){
		case FloatDeclaration:
		case IntegerDeclaration:
			token2 = scanner(source);
			if (strcmp(token2.tok, "f") == 0 ||
					strcmp(token2.tok, "i") == 0 ||
					strcmp(token2.tok, "p") == 0) {
				printf("Syntax Error: %s cannot be used as id\n", token2.tok);
				exit(1);
			}
			return makeDeclarationNode( token, token2 );
		default:
			printf("Syntax Error: Expect Declaration %s\n", token.tok);
			exit(1);
	}
}

Declarations *parseDeclarations( FILE *source )
{
	Token token = scanner(source);
	Declaration decl;
	Declarations *decls;
	int l;

	switch(token.type){
		case FloatDeclaration:
		case IntegerDeclaration:
			decl = parseDeclaration(source, token);
			decls = parseDeclarations(source);
			return makeDeclarationTree( decl, decls );
		case PrintOp:
		case Alphabet:
			l = strlen(token.tok);
			while (l > 0)
				ungetc(token.tok[--l], source);
			return NULL;
		case EOFsymbol:
			return NULL;
		default:
			printf("Syntax Error: Expect declarations %s\n", token.tok);
			exit(1);
	}
}

Expression *parseValue( FILE *source )
{
	Token token = scanner(source);
	Expression *value = (Expression *)malloc( sizeof(Expression) );
	value->leftOperand = value->rightOperand = NULL;

	switch(token.type){
		case Alphabet:
			(value->v).type = Identifier;
			strcpy((value->v).val.id, token.tok);
			break;
		case IntValue:
			(value->v).type = IntConst;
			(value->v).val.ivalue = atoi(token.tok);
			break;
		case FloatValue:
			(value->v).type = FloatConst;
			(value->v).val.fvalue = atof(token.tok);
			break;
		default:
			printf("Syntax Error: Expect Identifier or a Number %s\n", token.tok);
			exit(1);
	}

	Token next_token = scanner(source);
	Expression *expr;
	int l;
	
	switch (next_token.type) {
		case PlusOp:
		case MinusOp:
		case Alphabet:
		case PrintOp:
			l = strlen(next_token.tok);
			while (l > 0)
				ungetc(next_token.tok[--l], source);
		case EOFsymbol:
			break;
		case MulOp:
			expr = (Expression *)malloc( sizeof(Expression) );
			(expr->v).type = MulNode;
			(expr->v).val.op = Mul;
			expr->leftOperand = value;
			expr->rightOperand = parseValue(source);
			return expr;
		case DivOp:
			expr = (Expression *)malloc( sizeof(Expression) );
			(expr->v).type = DivNode;
			(expr->v).val.op = Div;
			expr->leftOperand = value;
			expr->rightOperand = parseValue(source);
			return expr;
		default:
			printf("Syntax Error: Expect Identifier or a Number %s\n", token.tok);
			exit(1);
	}
	return value;
}

Expression *parseExpressionTail( FILE *source, Expression *lvalue )
{
	Token token = scanner(source);
	Expression *expr;
	int l;

	switch(token.type){
		case PlusOp:
			expr = (Expression *)malloc( sizeof(Expression) );
			(expr->v).type = PlusNode;
			(expr->v).val.op = Plus;
			expr->leftOperand = lvalue;
			expr->rightOperand = parseValue(source);
			return parseExpressionTail(source, expr);
		case MinusOp:
			expr = (Expression *)malloc( sizeof(Expression) );
			(expr->v).type = MinusNode;
			(expr->v).val.op = Minus;
			expr->leftOperand = lvalue;
			expr->rightOperand = parseValue(source);
			return parseExpressionTail(source, expr);
		case Alphabet:
		case PrintOp:
			l = strlen(token.tok);
			while (l > 0)
				ungetc(token.tok[--l], source);
			return lvalue;
		case EOFsymbol:
			return lvalue;
		default:
			printf("Syntax Error: Expect a numeric value or an identifier %s\n", token.tok);
			exit(1);
	}
}

Expression *parseExpression( FILE *source, Expression *lvalue )
{
	Token token = scanner(source);
	Expression *expr;
	int l;

	switch(token.type){
		case PlusOp:
			expr = (Expression *)malloc( sizeof(Expression) );
			(expr->v).type = PlusNode;
			(expr->v).val.op = Plus;
			expr->leftOperand = lvalue;
			expr->rightOperand = parseValue(source);
			return parseExpressionTail(source, expr);
		case MinusOp:
			expr = (Expression *)malloc( sizeof(Expression) );
			(expr->v).type = MinusNode;
			(expr->v).val.op = Minus;
			expr->leftOperand = lvalue;
			expr->rightOperand = parseValue(source);
			return parseExpressionTail(source, expr);
		case Alphabet:
		case PrintOp:
			l = strlen(token.tok);
			while (l > 0)
				ungetc(token.tok[--l], source);
			return NULL;
		case EOFsymbol:
			return NULL;
		default:
			printf("Syntax Error: Expect a numeric value or an identifier %s\n", token.tok);
			exit(1);
	}
}

Statement parseStatement( FILE *source, Token token )
{
	Token next_token;
	Expression *value, *expr;

	switch(token.type){
		case Alphabet:
			next_token = scanner(source);
			if(next_token.type == AssignmentOp){
				value = parseValue(source);
				expr = parseExpression(source, value);
				return makeAssignmentNode(token.tok, value, expr);
			}
			else{
				printf("Syntax Error: Expect an assignment op %s\n", next_token.tok);
				exit(1);
			}
		case PrintOp:
			next_token = scanner(source);
			if(next_token.type == Alphabet)
				return makePrintNode(next_token.tok);
			else{
				printf("Syntax Error: Expect an identifier %s\n", next_token.tok);
				exit(1);
			}
			break;
		default:
			printf("Syntax Error: Expect a statement %s\n", token.tok);
			exit(1);
	}
}

Statements *parseStatements( FILE * source )
{

	Token token = scanner(source);
	Statement stmt;
	Statements *stmts;

	switch(token.type){
		case Alphabet:
		case PrintOp:
			stmt = parseStatement(source, token);
			stmts = parseStatements(source);
			return makeStatementTree(stmt , stmts);
		case EOFsymbol:
			return NULL;
		default:
			printf("Syntax Error: Expect statements %s\n", token.tok);
			exit(1);
	}
}


/*********************************************************************
  Build AST
 **********************************************************************/
Declaration makeDeclarationNode( Token declare_type, Token identifier )
{
	Declaration tree_node;

	switch(declare_type.type){
		case FloatDeclaration:
			tree_node.type = Float;
			break;
		case IntegerDeclaration:
			tree_node.type = Int;
			break;
		default:
			break;
	}
	strcpy(tree_node.name, identifier.tok);

	return tree_node;
}

Declarations *makeDeclarationTree( Declaration decl, Declarations *decls )
{
	Declarations *new_tree = (Declarations *)malloc( sizeof(Declarations) );
	new_tree->first = decl;
	new_tree->rest = decls;

	return new_tree;
}


Statement makeAssignmentNode( char *id, Expression *v, Expression *expr_tail )
{
	Statement stmt;
	AssignmentStatement assign;

	stmt.type = Assignment;
	strcpy(assign.id, id);
	if(expr_tail == NULL)
		assign.expr = v;
	else
		assign.expr = expr_tail;
	stmt.stmt.assign = assign;

	return stmt;
}

Statement makePrintNode( char *id )
{
	Statement stmt;
	stmt.type = Print;
	strcpy(stmt.stmt.variable, id);

	return stmt;
}

Statements *makeStatementTree( Statement stmt, Statements *stmts )
{
	Statements *new_tree = (Statements *)malloc( sizeof(Statements) );
	new_tree->first = stmt;
	new_tree->rest = stmts;

	return new_tree;
}

/* parser */
Program parser( FILE *source )
{
	Program program;

	program.declarations = parseDeclarations(source);
	program.statements = parseStatements(source);

	return program;
}


/********************************************************
  Build symbol table
 *********************************************************/
void InitializeTable( SymbolTable *table )
{
	int i;

	for(i = 0 ; i < 26; i++)
		table->table[i] = Notype;
}
/*
void add_table( SymbolTable *table, char c, DataType t )
{
	int index = (int)(c - 'a');

	if(table->table[index] != Notype)
		printf("Error : id %c has been declared\n", c);//error
	table->table[index] = t;
}
*/
void add_table(SymbolTable *table, char *c, DataType t)
{
	for (int i = 0; i < 26; i++) {
		if (table->table[i] == Notype) {
			table->table[i] = t;
			strcpy(table->name[i], c);
			return;
		}
	}
	printf("Error : Too many declaration.\n");//error
}

SymbolTable build( Program program )
{
	SymbolTable table;
	Declarations *decls = program.declarations;
	Declaration current;

	InitializeTable(&table);

	while(decls !=NULL){
		current = decls->first;
		add_table(&table, current.name, current.type);
		decls = decls->rest;
	}

	return table;
}


/********************************************************************
  Type checking
 *********************************************************************/

void convertType( Expression * old, DataType type )
{
	if(old->type == Float && type == Int){
		printf("error : can't convert float to integer\n");
		return;
	}
	if(old->type == Int && type == Float){
		Expression *tmp = (Expression *)malloc( sizeof(Expression) );
		if(old->v.type == Identifier)
			printf("convert to float %s \n",old->v.val.id);
		else
			printf("convert to float %d \n", old->v.val.ivalue);
		tmp->v = old->v;
		tmp->leftOperand = old->leftOperand;
		tmp->rightOperand = old->rightOperand;
		tmp->type = old->type;

		Value v;
		v.type = IntToFloatConvertNode;
		v.val.op = IntToFloatConvert;
		old->v = v;
		old->type = Int;
		old->leftOperand = tmp;
		old->rightOperand = NULL;
	}
}

DataType generalize( Expression *left, Expression *right )
{
	if(left->type == Float || right->type == Float){
		printf("generalize : float\n");
		return Float;
	}
	printf("generalize : int\n");
	return Int;
}

/*
DataType lookup_table( SymbolTable *table, char c )
{
	int id = c-'a';
	if( table->table[id] != Int && table->table[id] != Float)
		printf("Error : identifier %c is not declared\n", c);//error
	return table->table[id];
}
*/

int lookup_table(SymbolTable *table, char *c)
{
	int id;
	for (int i = 0; i < 26; i++) {
		if (strcmp(table->name[i], c) == 0) {
			id = i;
			break;
		}
	}
	if( table->table[id] != Int && table->table[id] != Float)
		printf("Error : identifier %s is not declared\n", c);//error
	return id;
}

void checkexpression( Expression * expr, SymbolTable * table )
{
	char *c;
	if(expr->leftOperand == NULL && expr->rightOperand == NULL){
		switch(expr->v.type){
			case Identifier:
				c = expr->v.val.id;
				printf("identifier : %s\n",c);
				expr->type = table->table[lookup_table(table, c)];
				break;
			case IntConst:
				printf("constant : int\n");
				expr->type = Int;
				break;
			case FloatConst:
				printf("constant : float\n");
				expr->type = Float;
				break;
				//case PlusNode: case MinusNode: case MulNode: case DivNode:
			default:
				break;
		}
	}
	else{
		Expression *left = expr->leftOperand;
		Expression *right = expr->rightOperand;

		checkexpression(left, table);
		checkexpression(right, table);

		DataType type = generalize(left, right);
		convertType(left, type);//left->type = type;//converto
		convertType(right, type);//right->type = type;//converto
		expr->type = type;
	}
}

void checkstmt( Statement *stmt, SymbolTable * table )
{
	if(stmt->type == Assignment){
		AssignmentStatement assign = stmt->stmt.assign;
		printf("assignment : %s \n",assign.id);
		checkexpression(assign.expr, table);
		stmt->stmt.assign.type = table->table[lookup_table(table, assign.id)];
		if (assign.expr->type == Float && stmt->stmt.assign.type == Int) {
			printf("error : can't convert float to integer\n");
		} else {
			convertType(assign.expr, stmt->stmt.assign.type);
		}
	}
	else if (stmt->type == Print){
		printf("print : %s \n",stmt->stmt.variable);
		lookup_table(table, stmt->stmt.variable);
	}
	else printf("error : statement error\n");//error
}

void check( Program *program, SymbolTable * table )
{
	Statements *stmts = program->statements;
	while(stmts != NULL){
		checkstmt(&stmts->first,table);
		stmts = stmts->rest;
	}
}


/***********************************************************************
  Code generation
 ************************************************************************/
void fprint_op( FILE *target, ValueType op )
{
	switch(op){
		case MinusNode:
			fprintf(target,"-\n");
			break;
		case PlusNode:
			fprintf(target,"+\n");
			break;
		case MulNode:
			fprintf(target,"*\n");
			break;
		case DivNode:
			fprintf(target,"/\n");
			break;
		default:
			fprintf(target,"Error in fprintf_op ValueType = %d\n",op);
			break;
	}
}

void fprint_expr( FILE *target, Expression *expr, SymbolTable *table)
{

	if(expr->leftOperand == NULL){
		switch( (expr->v).type ){
			case Identifier:
				fprintf(target,"l%c\n",lookup_table(table, (expr->v).val.id) + 'a');
				break;
			case IntConst:
				fprintf(target,"%d\n",(expr->v).val.ivalue);
				break;
			case FloatConst:
				fprintf(target,"%f\n", (expr->v).val.fvalue);
				break;
			default:
				fprintf(target,"Error In fprint_left_expr. (expr->v).type=%d\n",(expr->v).type);
				break;
		}
	}
	else{
		fprint_expr(target, expr->leftOperand, table);
		if(expr->rightOperand == NULL){
			fprintf(target,"5k\n");
		}
		else{
			//	fprint_right_expr(expr->rightOperand);
			fprint_expr(target, expr->rightOperand, table);
			fprint_op(target, (expr->v).type);
		}
	}
}

void gencode(Program prog, FILE * target, SymbolTable *table)
{
	Statements *stmts = prog.statements;
	Statement stmt;

	while(stmts != NULL){
		stmt = stmts->first;
		switch(stmt.type){
			case Print:
				fprintf(target,"l%c\n", lookup_table(table, stmt.stmt.variable) + 'a');
				fprintf(target,"p\n");
				break;
			case Assignment:
				fprint_expr(target, stmt.stmt.assign.expr, table);
				/*
				   if(stmt.stmt.assign.type == Int){
				   fprintf(target,"0 k\n");
				   }
				   else if(stmt.stmt.assign.type == Float){
				   fprintf(target,"5 k\n");
				   }*/
				fprintf(target,"s%c\n",lookup_table(table, stmt.stmt.assign.id) + 'a');
				fprintf(target,"0 k\n");
				break;
		}
		stmts=stmts->rest;
	}

}


/***************************************
  For our debug,
  you can omit them.
 ****************************************/
void print_expr(Expression *expr)
{
	if(expr == NULL)
		return;
	else{
		print_expr(expr->leftOperand);
		switch((expr->v).type){
			case Identifier:
				printf("%s ", (expr->v).val.id);
				break;
			case IntConst:
				printf("%d ", (expr->v).val.ivalue);
				break;
			case FloatConst:
				printf("%f ", (expr->v).val.fvalue);
				break;
			case PlusNode:
				printf("+ ");
				break;
			case MinusNode:
				printf("- ");
				break;
			case MulNode:
				printf("* ");
				break;
			case DivNode:
				printf("/ ");
				break;
			case IntToFloatConvertNode:
				printf("(float) ");
				break;
			default:
				printf("error ");
				break;
		}
		print_expr(expr->rightOperand);
	}
}

void test_parser( FILE *source )
{
	Declarations *decls;
	Statements *stmts;
	Declaration decl;
	Statement stmt;
	Program program = parser(source);

	decls = program.declarations;

	while(decls != NULL){
		decl = decls->first;
		if(decl.type == Int)
			printf("i ");
		if(decl.type == Float)
			printf("f ");
		printf("%s ",decl.name);
		decls = decls->rest;
	}

	stmts = program.statements;

	while(stmts != NULL){
		stmt = stmts->first;
		if(stmt.type == Print){
			printf("p %s ", stmt.stmt.variable);
		}

		if(stmt.type == Assignment){
			printf("%s = ", stmt.stmt.assign.id);
			print_expr(stmt.stmt.assign.expr);
		}
		stmts = stmts->rest;
	}

}
