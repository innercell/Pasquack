#include "lexico.h"
#include "gerador.h"

/*** Estrutura onde ficam as variaveis e seus tipos ***/
typedef struct var {
    int type;
    char *name;
    int var_pos;
    struct var *prox;
} variable;


/*** Prototipa��o de Fun��es ***/
void def_tipo_var(int ini,int fin,char *tipo);
int match(int tag);
int program();
int declaracoes();
int bloco();
int nome();
int constantes();
int variaveis();
int procedimentos();
int comandos();
int ident();
int lista_constantes();
int lista_ident();
int procedimento();
int comando();
int inteiro();
int parametros();
int lista_argumentos();
int expr_relacional();
int else_opc();
int expr();
int exprl();
int chamada_proc();
int op_relacional();
int termo();
int termol();
int fator();
int declaracao();
int uses();
int literal();
int tipo();
void gerLstVar();
void alocaVar(char *nome, int tipo);

/*** Variaveis Globais ***/
char fileName[20];
char *programName;
FILE *file;
FILE *gerFile;
int flagDeclara = 0;
int n_var = 0;
int flagName = 0;
int ignoraID = 0;
int tagfail;
int contVar = 0;
char *name_result;
variable *priVar;
struct Token * lastlook; ///Armazena o ultimo look antes de chamar novamente a fun��o scan, utilizado para gera��o de c�digo
char finalasm[1000]; ///Armazena o arquivo de sa�da
char instr[100]; ///armazena o comando que ser� inserido no arquivo de sa�da

/*** Fun��o Principal ***/
int main() {
    ini_symbol_table();
    printf("Digite o nome do arquivo a ser compilado\n");
    scanf("%s",fileName);
    file = fopen(fileName,"r");
    lookahead = (struct Token*) malloc(sizeof(struct Token));
    lookahead = scan(file);
    priVar = (struct var*) malloc(sizeof(struct var));
    priVar->prox = NULL;
    contVar = 0;
    if(program()) {
        printf("----------Codigo correto!------------\n");
        printf("%s\n",finalasm);
    } else {
        printf("**REAL: %10s[%i] | EXPECTED: %i\n",lookahead->lexeme,lookahead->tag,tagfail);
        printf("Codigo incorreto!");
    }
    fclose(gerFile);
    printTable();
    //system("pause");
    return 0;
}

/*** Fun��o que confere um programa ***/
int program() {
    if(declaracoes()) {
        if(bloco()) {
            if(match(EOF)) {
                return 1;
            } else return 0;
        } else return 0;
    } else return 0;
}

/*** Fun��o que confere a parte de declara��es ***/
int declaracoes() {
    if(nome()) {
        if(uses()) {
            if(constantes()) {
                if(variaveis()) {
                    if(procedimentos()) {
                        return 1;
                    } else return 0;
                } else return 0;
            } else return 0;
        } else return 0;
    } else return 0;
}

/*** Fun��o que o uso de bibliotecas ***/
int uses() {
    if(match(USES)) {
        if(ident()) {
            if(match(PONTOVIRGULA)) {
                return 1;
            } else return 0;
        } else return 0;
    }
    return 1;
}

/*** Fun��o que confere um bloco ***/
int bloco() {
    if(match(BEGIN)) {
        if(comandos()) {
            if(match(END)) {
                return 1;
            } else return 0;
        } else return 0;
    } else return 0;
}

/*** Fun��o que confere o nome do programa ***/
int nome() {
    if(match(PROGRAM)) {
        programName = lookahead->lexeme;
        flagName = 1;
        if(ident()) {
            flagName = 0;
            if(match(PONTOVIRGULA)) {
                name_result = strtok(fileName, ".");
                strcat(name_result,".asm");
                gerFile = fopen(name_result, "wb+");
                beginGereration(programName,gerFile);
                header(gerFile);
                return 1;
            } else return 0;
        } else return 0;
    } else return 1;
}

/*** Fun��o que confere declara��o de constantes ***/
int constantes() {
    if(match(CONSTANTE)) {
        if(lista_constantes()) {
            return 1;
        } else return 0;
    } else return 1;
}

/*** Fun��o que confere declara��o de variaveis ***/
int variaveis() {
    int var_ini,var_fin;
    char *tipo_v;
    while(match(VAR)) {
        flagDeclara = 1;
        var_ini = n_var;
        if(lista_ident()) {
            var_fin = n_var;
            flagDeclara = 0;
            if(match(DOISPONTOS)) {
                tipo_v = lookahead->lexeme;
                def_tipo_var(var_ini,var_fin,tipo_v);
                if(tipo()) {
                    if(match(PONTOVIRGULA)) {
                        continue;
                    } else return 0;
                } else return 0;
            } else if(match(PONTOVIRGULA)) {
                return 1;
            } else return 0;
        } else return 0;
    }
    gerLstVar();
    return 1;
}

/*** Fun��o que confere o tipo da variavel ***/
int tipo() {
    if(match(INTEGER)||match(STRING)||match(REAL)||match(BOOLEAN)||match(CHAR)||match(LONGINT)||match(SHORTINT)) {
        return 1;
    } else return 0;
}

/*** Fun��o que confere um procedimento ***/
int procedimentos() {
    if(procedimento()) {
        if(match(PONTOVIRGULA)) {
            if(procedimentos()) {
                return 1;
            } else return 0;
        } else return 0;
    } else return 1;
}

/*** Fun��o que confere os comandos ***/
int comandos() {
    if(comando()) {
        if(match(PONTOVIRGULA)) {
            if(comandos()) {
                return 1;
            } else return 0;
        } else return 1;
    } else return 1;
}

/*** Fun��o que confere um identificador ***/
int ident() {
    if(lookahead->lexeme == programName && flagName != 1) {
        printf("erro, nome do programa n�o pode ser usado como Variavel");
        exit(0);
    } else if(match(ID)) {
        return 1;
    } else{return 0;}
}

/*** Fun��o que confere a lista de constantes ***/
int lista_constantes() {
    if(ident(id)) {
        if(match(EQUAL)) {
            if(inteiro()) {
                if(match(VIRGULA)) {
                    if(lista_constantes()) {
                        return 1;
                    } else return 0;
                } else return 1;
            } else return 0;
        } else return 0;
    } else return 0;
}

/*** Fun��o que confere a lista de Identificadores ***/
int lista_ident() {
    char *nome;
    if(flagDeclara == 1) {
        nome = lookahead->lexeme;
        alocaVar(nome,0);
        n_var ++;
    }
    if(ident()) {
        if(match(VIRGULA)) {
            if(lista_ident()) {
                return 1;
            } else if(lookahead->tag==ASPAS) {
                return 1;
            } else return 0;
        } else return 1;
    } else if(lookahead->tag==ASPAS) {
        return 1;
    } else return 0;
}

/*** Fun��o que confere um procedimento ***/
int procedimento() {
    if(match(PROCEDURE)) {
        if(ident()) {
            if(parametros()) {
                if(declaracao()) {
                    if(bloco()) {
                        return 1;
                    } else return 0;
                } else return 0;
            } else return 0;
        } else return 0;
    } else return 0;
}

/*** Fun��o que confere um comando ***/
int comando() {
    if(match(PRINT)) {
        if(match(LPARENT)) {
            if(lista_argumentos()) {
                if(match(RPARENT)) {
                    return 1;
                } else return 0;
            } else return 0;
        } else return 0;
    } else if(match(IF)) {
        if(match(NOT)) return 1;
        if(expr_relacional()) {
            if(match(OR)||match(AND)) return 1;
            if(match(THEN)) {
                if(comando()) {
                    if(else_opc()) {
                        return 1;
                    } else return 0;
                } else return 0;
            } else return 0;
        } else if(match(LPARENT)) {
            if(expr_relacional()) {
                if(match(RPARENT)) {
                    if(match(OR)||match(AND)) return 1;
                    if(match(THEN)) {
                        if(comando()) {
                            if(else_opc()) {
                                return 1;
                            } else return 0;
                        } else return 0;
                    } else return 0;
                } else return 0;
            } else return 0;
        } else return 0;
    } else if(ident()) {
        char id[20];
        strcpy(id,lastlook->lexeme);
        printf("lastlex=%s\n",lastlook->lexeme);system("pause");
        if(match(ASSIGN)) {
            sprintf(instr,"mov [%s],",id);
            printf("mov [%s],",id);
            strcat(finalasm,instr);
            if(expr()) {
                if(match(PONTOVIRGULA)) {
                    if(comando()) {
                        if(comandos()) return 1;
                        return 1;
                    } else return 1;
                } else return 0;
            } else if(match(NUM)) {
                sprintf(instr,"%d\n",lastlook->value);
                printf("num%d\n",lastlook->value);
                strcat(finalasm,instr);
                return 1;
            } else return 0;
        } else return 0;
    } else if(match(FOR)) {
        if(ident()) {
            if(match(ASSIGN)) {
                if(expr()) {
                    if(match(TO) || match(DOWNTO)) {
                        if(expr()) {
                            if(match(DO)) {
                                if(comando()) {
                                    if(comandos()) return 1;
                                    return 1;
                                } else return 0;
                            } else return 0;
                        } else return 0;
                    } else return 0;
                } else return 0;
            } else return 0;
        } else return 0;
    } else if(chamada_proc()) {
        return 1;
    } else if(bloco()) {
        return 1;
    } else if(match(WRITELN)||match(WRITE)) {
        if(match(LPARENT)) {
            if(literal()) {
                while(match(VIRGULA)) {
                    if(expr()) {
                        continue;
                    } else if(literal()) {
                        continue;
                    } else if(expr()) {
                        return 1;
                    } else {
                        return 0;
                    }
                }
                if(match(RPARENT)) {
                    return 1;
                } else return 0;
            } else if(ident()) {
                while(match(VIRGULA)) {
                    if(ident()) {
                        continue;
                    } else if(literal()) {
                        continue;
                    } else if(expr()) {
                        return 1;
                    } else {
                        return 0;
                    }
                }
                if(match(RPARENT)) {
                    return 1;
                } else return 0;
            } else if(match(RPARENT)) {
                return 1;
            } else return 0;
        } else return 0;
    } else if(match(READLN)) {
        if(match(LPARENT)) {
            if(lista_ident()) {
                if(match(RPARENT)) {
                    return 1;
                } else return 0;
            } else return 0;
        } else return 0;
    } else if(match(BREAK)||match(READKEY)) {
        if(match(PONTOVIRGULA)) {
            return 1;
        } else return 0;
    } else if(match(WHILE)) {
        if(match(LPARENT)) {
            if(expr_relacional()) {
                if(match(RPARENT)) {
                    if(match(DO)) {
                        if(comando()) {
                            if(comandos()) return 1;
                            return 1;
                        } else return 0;
                    } else return 0;
                } else return 0;
            } else return 0;
        } else if(expr_relacional()) {
            if(match(DO)) {
                if(comando()) {
                    return 1;
                } else return 0;
            } else return 0;
        } else return 0;
    } else return 1;
}

/*** Fun��o que confere um literal ***/
int literal() {
    if(match(ASPAS)) {
        if(match(LITERAL)) {
            if(match(ASPAS)) {
                return 1;
            } else return 0;
        } else return 0;
    } else if(match(283)) {
        return 1;
    } else {
        return 0;
    }
}

/*** Fun��o que confere um inteiro ***/
int inteiro() {
    if(match(NUM)) {
        sprintf(instr,"%d\n",lastlook->value);
        printf("%d\n",lastlook->value);
        strcat(finalasm,instr);
        return 1;
    } else {
        return 0;
    }
}

/*** Fun��o que confere um parametro ***/
int parametros() {
    //printf("PARAMETROS\n");
    if(match((LPARENT))) {
        if(lista_ident()) {
            if(match(RPARENT)) {
                return 1;
            } else return 0;
        } else return 0;
    } else return 1;
}

/*** Fun��o que confere a lista de argumentos ***/
int lista_argumentos() {
    //printf("LISTA_ARGUMENTOS\n");
    if(expr()) {
        if(match(VIRGULA)) {
            if(lista_argumentos()) {
                return 1;
            }
        } else return 1;
    }
    return 0;
}

/*** Fun��o que confere uma expressao relacional ***/
int expr_relacional() {
    //printf("EXPR_RELACIONAL\n");
    if(expr()) {
        if(op_relacional()) {
            if(expr()) {
                return 1;
            } else return 0;
        } else return 0;
    } else return 0;
}

/*** Fun��o que confere o comando else ***/
int else_opc() {
    //printf("ELSE_OPC\n");
    if(match(ELSE)) {
        if(comando()) {
            return 1;
        } else return 0;
    } else return 1;
}

/*** Fun��o que confere uma expressao ***/
int expr() {
    //printf("EXPR\n");
    if(termo()) {
        if(exprl()) {
            return 1;
        } else return 1;
    } else return 0;
}

/*** Fun��o que confere a expressao linha(usado pra remover recurs�o a esquerda) ***/
int exprl() {
    //printf("EXPRL\n");
    if(match(PLUS)) {
        if(termo()) {

            if(exprl()) {
                return 1;
            } else return 0;
        } else return 0;
    } else if(match(MINUS)) {
        if(termo()) {
            if(exprl()) {
                return 1;
            } else return 0;
        } else return 0;
    } else return 1;
}

/*** Fun��o que confere uma chamada de procedimentos ***/
int chamada_proc() {
    //printf("CHAMADA_PROC\n");
    if(ident()) {
        if(lista_argumentos()) {
            return 1;
        } else return 1;
    } else return 0;
}

/*** Fun��o que confere um operador relacional ***/
int op_relacional() {
    //printf("OP_RELACIONAL\n");
    if(match(EQUAL)) {
        return 1;
    } else if(match(NEQUAL)) {
        return 1;
    } else if(match(LEQUAL)) {
        return 1;
    } else if(match(GEQUAL)) {
        return 1;
    } else if(match(LTHAN)) {
        return 1;
    } else if(match(GTHAN)) {
        return 1;
    } else return 0;
}

/*** Fun��o que confere um termo ***/
int termo() {
    //printf("TERMO\n");
    if(fator()) {
        if(termol()) {
            return 1;
        } else return 0;
    } else return 0;
}

/*** Fun��o que confere um termo linha(usado pra remover recurs�o a esquerda) ***/
int termol() {
    //printf("TERMOL\n");
    if(match(MULTI)) {
        if(fator()) {
            if(termol()) {
                return 1;
            } else return 1;
        } else return 0;
    } else if(match(DIVIDE)||match(DIV)||match(MOD)) {
        if(fator()) {
            if(termol()) {
                return 1;
            } else return 1;
        } else return 0;
    } else return 1;
}

/*** Fun��o que confere um fator ***/
int fator() {
    if(match(LPARENT)) {
        if(expr()) {
            if(match(RPARENT)) {
                return 1;
            } else return 0;
        }
    } else if(ident()) {
        return 1;
    } else if(inteiro()) {
        return 1;
    }
    return 0;
}

/*** Fun��o que confere uma declara��o ***/
int declaracao() {
    //printf("DECLARACAO\n");
    if(constantes()) {
        if(variaveis()) {
            return 1;
        } else return 0;
    }
    return 0;
}

/*** Fun��o de compara��o do token recebido pelo lexo com o token possivel ***/
int match(int tag) {
    //printf("**REAL: %10s[%i] | EXPECTED: %i\n",lookahead->lexeme,lookahead->tag,tag);
    if(tag==lookahead->tag) {
        lastlook=lookahead;
        printf("Encontrado %s\n",lookahead->lexeme);
        lookahead = (struct Token*) malloc(sizeof(struct Token));
        lookahead = scan(file);
        return 1;
    } else {
        //printf("Lexema nao encontrado!\n");
        tagfail = tag;
        return 0;
    }
}

/*** Fun��o que chama o gerador para gerar o codigo das variaveis ***/
void gerLstVar() {
    variable *aux;
    aux = priVar;
    do {
        char saux[100];
        strcpy (saux,"\t");
        strcat(saux,aux->name);
        if(aux->type == 1) {
            dotData(gerFile,saux," db \n");
        } else if(aux->type == 2) {
            dotData(gerFile,saux," db \n");
        } else if(aux->type == 3) {
            dotData(gerFile,saux," dw \n");
        } else if(aux->type == 4) {
            dotData(gerFile,saux," dw \n");
        } else if(aux->type == 5) {
            dotData(gerFile,saux," dw \n");
        }
        free(saux);
        aux = aux->prox;
    } while(aux != NULL);
}

/*** Fun��o que aloca e preenche as variavies ***/
void alocaVar(char *nome, int tipo) {
    variable *aux,*novo;
    aux = priVar;
    if(contVar == 0) {
        priVar->name = nome;
        priVar->type = tipo;
        priVar->var_pos = n_var;
        contVar++;
    } else {
        while(aux->prox!=NULL) {
            aux=aux->prox;
        }
        novo = (struct var*) malloc(sizeof(struct var));
        novo->name = nome;
        novo->type = tipo;
        novo->prox = NULL;
        novo->var_pos = n_var;
        aux->prox = novo;
        contVar++;
    }
}
void def_tipo_var(int ini,int fin,char *tipo) {
    variable *aux = priVar;
    int tipo_aux;
    if(!strcmp(tipo,"shortint")) {
        tipo_aux = 1;
    } else if(!strcmp(tipo,"integer")) {
        tipo_aux = 2;
    } else if(!strcmp(tipo,"longint")) {
        tipo_aux = 3;
    } else if(!strcmp(tipo,"real")) {
        tipo_aux = 4;
    } else if(!strcmp(tipo,"char")) {
        tipo_aux = 5;
    }
    while(aux->var_pos < ini) {
        aux = aux->prox;
    }
    while(aux!=NULL) {
        aux->type =tipo_aux;
        aux = aux->prox;
    }
}

