/* 
 * UC:21111 - Sistemas Operativos - Universidade Aberta
 * e-fólio A 2023-24 (fdup.c)
 *
 * Aluno: 1900249 - Sónia Costa
 */

#include <stdio.h> // Entrada/saidas
#include <stdlib.h>
#include <unistd.h> // Para usar funcões do sistema operacional como a função fork()
#include <sys/wait.h> // Para a função waitpid()


/**
 * Verificar se o número de argumentos inseridos pelo utilizador é pelos 3,  
 * 3 argumentos pois o nome do comando conta como um argumento, mais a directoria (~/Doc), e o nome dp ficheiro, pelo que temos de contabilizar 3
 */
int validar_args ( int argc ) { 
    if (argc != 3) {			
        printf ("ERRO: número de argumentos incorrecto. Tem de ser pelo menos 3.\n");  
        return 0;     
    }
    else {
        return 1;      
    }
}

/**
 * Validar se existe a directoria de entrada
 */
int validar_directoria (char *argv[]) { 				
    FILE *file;
    file = fopen(argv[1], "r");
    if (file) {
        fclose(file);
        printf("Directória existente: %s\n",argv[1]);
        return 1;
    } 
    else {
        printf("Diretória inválida ou inexistente: %s\n",argv[1]);
        return 0;
    }
}

/**
 * Verifica se criou um novo processo filho
 */
int verificar_fork(pid_t pid, char ch) {
    if ( pid < 0 ) {
        printf ("Erro ao gerar filho (erro na função fork()) - Processo %c.", ch);
        return 0;
    }
    else {
        return 1;
    }     
}

/**
 * Imprime para o ecrâ os dados de um processo, mais o PID e o PPID.
 */
void print_msg_processo(char ch) {
   printf("Processo %c: PID=%5d  PPID=%5d\n", ch, (int) getpid(), (int) getppid());
}

/**
 * Impimir uma mensagem erro quando dá erro na execução de um dos processos (A, B, C ou D)
 */
void print_msg_erro_no_processo (char ch) {
    printf("Ocorreu um erro no processo %c.",ch);    
}

/**
 * Primeiro aponta-se o stdout para o ficheiro tmp1.txt, e depois executa-se a troca da imagem com o execl 
 * -> Passo 1: $ find ~Doc -type -name '*.c' -print "%h %f\n" >tmp1.txt
 */
void processo_p1(char *argv[]) {    
    freopen("tmp1.txt", "w+", stdout);
    //execl("/usr/bin/find", "find", "~/Doc", "-type", "f", "-name", "'*.c'", "-printf", "%h %f\n", NULL);
    /**
     * Não pode usar a solução acima comentada, então usei a solução abaixo. 
     * Pela minha pesquisa tive de usar esta solução porque por alguma razão a shell (sh ou bash) não foi encontrada.
     */
    execl("/bin/sh", "sh", "-c", "find ~/Doc -type f -name '*.c' -printf \"%h %f\n\"", NULL);
    perror("execl - Processo B");
    print_msg_erro_no_processo('B');
}  

/**
 *  
 * Aponta-se o stdin para o ficheiro tmp1.txt e para o tmp2.txt stdout,
 * e depois ordena as linha contidas no fichiro tmp1.txt e escreve o resultado no ficheiro tmp2.txt
 * -> Passo 2: $ sort -t ' ' -k2,2 <tmp1.txt >tmp2.txt
 */
void processo_p2() {
    freopen("tmp1.txt", "r", stdin); // reabrir o ficheiro a ler 
    freopen("tmp2.txt", "w+", stdout); // reabrir o ficheiro a escrever
    char* args[] = {"sort", "-t", " ", "-k2,2", NULL};
    execv("/bin/sort", args);
    perror("execv - Processo C");
    print_msg_erro_no_processo('C');
}

/**
 *  
 * Aponta-se o stdin para o ficheiro tmp2.txt e para o tmp3.txt o stdout,
 * e depois lista as linhas repetidas no tmp2.txt e escreve o resultado no ficheiro tmp3.txt
 * -> Passo 3: $ uniq -f1 -D
 */
void processo_p3() {
    freopen("tmp2.txt", "r", stdin); // reabrir o ficheiro a ler
    freopen("tmp3.txt", "w+", stdout); // reabrir o ficheiro a escrever
    execlp( "/bin/uniq", "uniq", "-f1", "-D", NULL);
    perror("execlp - Processo D");
    print_msg_erro_no_processo('D');
}

/**
 * Troca a posição dos 2 campos em cada linha do ficheiro tmp3.text
 * e imprime o resultado final na forma de nome de ficheiro e respectiva pathname em cada linha
 * -> Passo 4: $ awk '{print $2, $1}' <tpm3.txt
 */
void processo_p4() {
    freopen("tmp3.txt", "w+", stdin); // reabrir o ficheiro a ler   
    char* args[] = {"awk", "{print $2 $1}" , NULL};
    execvp("/usr/bin/awk", args); 
    perror("execvp - Processo E");
    print_msg_erro_no_processo('E');
}

/**
 * É a partir do processo A que serão derivados os 4 processos sequenciais gerados pelo mesmo pai (Processo A).
 */
int main(int argc, char** argv) {
	
    pid_t pidB,pidC,pidD,pidE;
    int v1, v2, vf_b, vf_c, vf_d, vf_e;
    
    // Processo A (PAI)
    v1 = validar_args(argc);
    if (v1==0) return 0; // Se não validar sair do programa 
    
    v2 = validar_directoria(argv);
    if (v2==0) return 0; // Se não validar sair do programa

    pidB=fork();
	
    vf_b = verificar_fork(pidB, 'B');
    if (vf_b==0) return 0; // Se não validar sair do programa
        
    if ( pidB == 0 ) {
        // Processo B (FILHO)
        print_msg_processo('B');
        processo_p1(argv);             
    } 
    else{
        print_msg_processo('A');
	    waitpid(pidB,NULL,0); // O processo A aguarda pela terminação do processo filho, antes de ser criado o processo B.

	    pidC=fork();
		
	    vf_c = verificar_fork(pidC, 'C');
	    if (vf_c==0) return 0; // Se não validar sair do programa

	    if (pidC == 0) {
	    	// Processo C (FILHO)
		    print_msg_processo('C');
		    processo_p2();
	    } 
	    else{
		    waitpid(pidC,NULL,0); // Aguarda pela terminação do processo filho, antes de ser criado o processo C.

		    pidD=fork();
		    
		    vf_d = verificar_fork(pidD, 'D');
		    if (vf_d == 0) return 0;   

		    if (pidD == 0){
			    // Processo D (FILHO)
			    print_msg_processo('D');              
			    processo_p3();
		    } 
		    else{
    
			    waitpid(pidD,NULL,0); // Aguarda pela terminação do processo filho, antes de ser criado o processo D.

			    pidE=fork();

			    vf_e = verificar_fork(pidE, 'E');
			    
			    if (vf_e == 0) return 0; 

			    if (pidE == 0) {
				    // Processo E (FILHO)
				    print_msg_processo('E');                 
				    processo_p4();
			    } 
			    else{
			         waitpid(pidE,NULL,0); // Aguarda pela terminação do processo filho, antes de ser criado o processo E.                                          
                     //Termina processo A;
			   }
            }         
        }                  
    }  
         
    return 1;
}
