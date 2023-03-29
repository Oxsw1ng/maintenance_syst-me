#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <termios.h>

#define MAXLI 2048
#define MAX_ARGS 10

char cmd[MAXLI];
int commandLength = 0;
char *args[MAX_ARGS];
int i;
int current_command = 0;
int nbCommandes;
FILE *fp;
// 0 si invalide, 1 si valide et 2 si les flèches sont pressés (pour reprendre les anciennes commandes)
int caractereValide = 1;
// 1 si flèche du haut pressée, 0 si flèche du bas
int up = 1;

void mbash();
int parseCommand(char *command);
void clearScreen();
//int tabCompletion(char *commandPart);
int traiterTouchesSpe(char *touche, struct termios term);

int traiterTouchesSpe(char *touche, struct termios term) {
  // Permet de traiter les touches spéciales / raccourcis dans le bash
  if (*touche == 12) {
    clearScreen();
    *touche = '\n';
    return 0;
  } else if (*touche == 127) {
    if (commandLength > 0) {
      printf("\b \b");
      commandLength--;
      cmd[commandLength] = ' ';
    }
    caractereValide = 0;
    return 1;
  } else if (*touche == VEOF) {
    printf("\n\n FIN DU PROGRAMME\n\n");
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    exit(0);
    return 0;
  } else if (*touche == 65) {
    caractereValide=2;
    commandLength = commandLength-2;
    up = 1;
    return 1;
  } else if (*touche == 66) {
    commandLength = commandLength-2;
    caractereValide=2;
    up = 0;
    return 1;
  }  else if (*touche == '\n') {
    if (commandLength == 0) {
      printf("\n");
    }
    return 0;
  }
  return 1;
}

//int tabCompletion(char *commandPart) {
  // Permet de compléter la commande avec la touche tab
 // char* results[100];
  //int nbResults = 0;
  //char* smallestFileName = "lllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll";
 // char* dirname = ".";
  //DIR* dir = opendir(dirname);
  //struct dirent* entry;
  //while ((entry = readdir(dir)) != NULL) {
  //  if (strstr(entry->d_name, commandPart) != NULL) {
      //printf("%s\n", entry->d_name);
   //   results[nbResults] = entry->d_name;
    //  nbResults++;
   //   if (strlen(entry->d_name) < strlen(smallestFileName)) {
  //      smallestFileName = entry->d_name;
  //    }
 //   }
 // }
 // if (nbResults == 1) { 
 //   commandPart = results[0]; 
 // } else if (nbResults > 1) {
 //   for (int j = 0; j < nbResults; j++) {
 //     printf("%s\n", results[nbResults]);
 //   }
 // }
 // closedir(dir);
// return 0;
//}

void clearScreen() {
  printf("\033[2J\033[1;1H");
}

int main(int argc, char** argv) {
  // Déclaration des attriburs nécessaires à la méthode
  char ch;
  int chValide = 1;
  int iterateur = 0;
  char *home = getenv("HOME");
  char file_path[200];
  sprintf(file_path, "%s/.history", home);
  struct termios term, save;
  char ligne[MAXLI];

  nbCommandes = 0;
  
  if (access(file_path, F_OK) != -1) {
  fp = fopen(file_path, "r");
  if (fp != NULL) {
    while (fgets(ligne, sizeof(ligne), fp) != NULL) {
      nbCommandes++;
    }
  }
  fclose(fp);
  }


  
  while (1) {
    // On récupère le répertoire courant
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
      perror("getcwd() error");
    }
    // Puis on l'affiche en tant que prompt
    printf("%s > ",cwd);

    // On désactive la bufferisation et l'affichage pour pouvoir traiter les raccourcis (on réactivera l'affichage uniquement pour les caractères non spéciaux)
    tcgetattr(STDIN_FILENO, &term);
    save = term;
    term.c_lflag &= ~(ICANON | ECHO );
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    // On entre les caractères 1 par 1 jusqu'à que ce soit un caractère spécifique comme Entrée
    ch = getchar();

    while (traiterTouchesSpe(&ch, term)) {

      tcgetattr(STDIN_FILENO, &term); term.c_lflag &= ~ECHO; tcsetattr(STDIN_FILENO, TCSANOW, &term);

      // Si le caractère est "lambda"
      if (caractereValide == 1) {
        tcgetattr(STDIN_FILENO, &term); term.c_lflag |= ECHO; tcsetattr(STDIN_FILENO, TCSANOW, &term);
        printf("%c",ch);
        cmd[commandLength] = ch;
        commandLength++;
        tcgetattr(STDIN_FILENO, &term); term.c_lflag &= ~ECHO; tcsetattr(STDIN_FILENO, TCSANOW, &term);

      // Si les flèches sont pressées  
      } else if (caractereValide == 2) {

        tcgetattr(STDIN_FILENO, &term); term.c_lflag |= ECHO; tcsetattr(STDIN_FILENO, TCSANOW, &term);
        if (up == 1) {
          if (current_command > 0) {
            current_command--;
          }
        } else {
          if (current_command < nbCommandes) {
            current_command++;
          }
        }
        if (current_command >= 0 && current_command <= nbCommandes) {
        fp = fopen(file_path, "r");
        if (fp != NULL) {
          while (commandLength > 0) {
            cmd[commandLength] = ' ';
            commandLength--;
            printf("\b \b");
          }
          for (int k = 0; k <= current_command; k++) {
            fgets(ligne, sizeof(ligne), fp);
          }
          int j = 0;
          while (ligne[j+1] != '\0') {
            cmd[j] = ligne[j];
            printf("%c", cmd[j]);
            j++;
            commandLength++;
          }
          printf("\n\n %d \n", commandLength);
        }
        fclose(fp);
        }
        tcgetattr(STDIN_FILENO, &term); term.c_lflag &= ~ECHO; tcsetattr(STDIN_FILENO, TCSANOW, &term);
      }
      caractereValide = 1;
      ch = getchar();
    }
    tcgetattr(STDIN_FILENO, &term); term.c_lflag |= ECHO; tcsetattr(STDIN_FILENO, TCSANOW, &term);

    // On traite la commande si elle n'est pas NULL
    if (commandLength > 0) {
      printf("\nCommande : %s\n",cmd);
      fp = fopen(file_path, "a");
      if (fp != NULL) {
        fprintf(fp, "%s\n", cmd);
      } 
      fclose(fp);
      // Si la commande n'est pas "spéciale", il faut l'exécuter
      if (parseCommand(cmd) == 0) {
        mbash();
      }
      // Supprime l'ancienne commande et l'enregistre dans un tableau (pour history)
      while (iterateur < commandLength) {
        cmd[iterateur] = ' ';
        iterateur++;
      }
      // Reset des variables
      nbCommandes++;
      current_command = nbCommandes;
      iterateur = 0;
      commandLength = 0;
      chValide = 1;
    }
  }

  return 0;
  }

int parseCommand(char *command) {
  // Déclaration des attriburs nécessaires à la méthode
  char *home = getenv("HOME");
  char file_path[200];
  sprintf(file_path, "%s/.history", home);
  char ligne[MAXLI];
  i = 0;

  // On découpe la commande par espace pour chaque argument
  char *token = strtok(command, " ");
  while (token) {
    args[i++] = token;
    token = strtok(NULL, " ");
  }
  args[i] = NULL;
  // Traitement des commandes spéciales
  if (strcmp(args[0], "exit") == 0) {
    exit(0);
  }
  if (strcmp(args[0], "clear") == 0) {
    clearScreen();
    return 1;
  }
  if (strcmp(args[0], "cd") == 0) {
    if (args[1] != NULL) {
      if (chdir(args[1]) != 0) {
	      perror("ERROR");
        printf("erreur bien ici");
      } 
    } else {
      chdir(getenv("HOME"));
    }
    return 1;
  }
  if (strcmp(args[0], "history") == 0) {
      fp = fopen(file_path, "r");
      int iterator = 1;
      while (fgets(ligne, sizeof(ligne), fp) != NULL) {
        printf("%d : %s", iterator,ligne);
        iterator++;
      }
      fclose(fp);
      return 1;
  }
  return 0;

}

void mbash() {
  // fork() pour traiter les cas avec &
  pid_t pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("ERROR");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror ("ERROR");
  } else {
    if (strcmp(args[i-1], "&") != 0) {
      waitpid(pid, NULL, WUNTRACED);
    }
  }
}
