/*
 * ============================================================
 *   Projet C : Automate de Vente Intelligent
 *   Simulateur de distributeur automatique avec couleurs et admin
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

/* ============================================================
 *  CONSTANTES & STRUCTURES
 * ============================================================ */

#define NB_PRODUITS     4
#define AUTH_FILE       "auth.txt"
#define VENTE_FILE      "vente.dat"
#define MDP_MAX         50
#define LOGIN_MAX       50
#define MAX_TRANSACTIONS 100

/* Codes couleurs ANSI */
#define COLOR_RESET     "\033[0m"
#define COLOR_RED       "\033[31m"
#define COLOR_GREEN     "\033[32m"
#define COLOR_YELLOW    "\033[33m"
#define COLOR_BLUE      "\033[34m"
#define COLOR_MAGENTA   "\033[35m"
#define COLOR_CYAN      "\033[36m"
#define COLOR_WHITE     "\033[37m"
#define COLOR_BOLD      "\033[1m"

/* Pieces acceptees */
#define NB_PIECES       5
const double PIECES_VALIDES[NB_PIECES] = {0.10, 0.20, 0.50, 1.00, 2.00};

/* Structure pour une transaction */
typedef struct {
    char produit[40];
    double prix;
    double montant_insere;
    double monnaie_rendue;
    char date_heure[30];
} Transaction;

/* Produits du distributeur */
typedef struct {
    int    id;
    char   nom[40];
    double prix;
    int    nb_vendus;
} Produit;

/* Donnees globales de la machine */
typedef struct {
    Produit produits[NB_PRODUITS];
    double  chiffre_affaires;
    Transaction transactions[MAX_TRANSACTIONS];
    int nb_transactions;
} Machine;

/* ============================================================
 *  VARIABLES GLOBALES
 * ============================================================ */

Machine machine;
int     admin_initialise = 0;   /* 1 si compte admin deja cree */

/* ============================================================
 *  PROTOTYPES
 * ============================================================ */

void    initialiser_machine(void);
void    afficher_menu_client(void);
void    afficher_entete(void);
void    cycle_vente(int choix);
int     piece_valide(double piece);
double  arrondir(double val);
void    menu_administration(void);
int     authentifier_admin(void);
void    creer_compte_admin(void);
void    afficher_caisse(void);
void    afficher_rapport_ventes(void);
void    afficher_historique(void);
void    changer_mot_de_passe(void);
void    reinitialiser_machine(void);
void    sauvegarder_ventes(void);
void    charger_ventes(void);
void    vider_buffer(void);
void    afficher_transaction(Transaction t);
void    get_date_heure(char *buffer, int taille);
void    afficher_animation(const char *texte, int duree_ms);
void    nettoyer_ecran(void);

/* ============================================================
 *  MAIN
 * ============================================================ */

int main(void) {
    int choix;

    #ifdef _WIN32
        system("chcp 65001 > nul");  /* Support UTF-8 sur Windows */
    #endif

    initialiser_machine();
    charger_ventes();

    do {
        afficher_menu_client();
        printf(COLOR_CYAN "Saisissez votre choix : " COLOR_RESET);
        if (scanf("%d", &choix) != 1) {
            vider_buffer();
            printf(COLOR_RED "\n  [!] Saisie invalide. Veuillez entrer un nombre.\n\n" COLOR_RESET);
            continue;
        }
        vider_buffer();

        switch (choix) {
            case 1:
            case 2:
            case 3:
            case 4:
                cycle_vente(choix - 1);
                break;
            case 0:
                printf(COLOR_GREEN "\n  ? Merci d'avoir utilise notre distributeur ! A bientot ! ?\n\n" COLOR_RESET);
                break;
            case 999:
                menu_administration();
                break;
            default:
                printf(COLOR_RED "\n  [!] Choix invalide. Veuillez reessayer.\n\n" COLOR_RESET);
        }
    } while (choix != 0);

    return 0;
}

/* ============================================================
 *  INITIALISATION
 * ============================================================ */

void initialiser_machine(void) {
    int i;
    
    /* Produits par defaut */
    machine.produits[0] = (Produit){1, "Cafe Court",      0.40, 0};
    machine.produits[1] = (Produit){2, "The a la Menthe", 0.50, 0};
    machine.produits[2] = (Produit){3, "Chocolat Chaud",  0.60, 0};
    machine.produits[3] = (Produit){4, "Cappuccino",      0.80, 0};
    machine.chiffre_affaires = 0.0;
    machine.nb_transactions = 0;
    
    for (i = 0; i < MAX_TRANSACTIONS; i++) {
        machine.transactions[i].produit[0] = '\0';
    }

    /* Verifier si un compte admin existe deja */
    FILE *f = fopen(AUTH_FILE, "r");
    if (f != NULL) {
        admin_initialise = 1;
        fclose(f);
    }
}

/* ============================================================
 *  AFFICHAGE ENTETE
 * ============================================================ */

void afficher_entete(void) {
    printf(COLOR_BOLD COLOR_CYAN);
    printf("\n");
    printf("  +----------------------------------------------------------+\n");
    printf("  ¦                                                          ¦\n");
    printf("  ¦        ??  DISTRIBUTEUR AUTOMATIQUE INTELLIGENT  ??      ¦\n");
    printf("  ¦                                                          ¦\n");
    printf("  +----------------------------------------------------------+\n");
    printf(COLOR_RESET);
}

/* ============================================================
 *  INTERFACE CLIENT
 * ============================================================ */

void afficher_menu_client(void) {
    int i;
    
    nettoyer_ecran();
    afficher_entete();
    
    printf("\n");
    printf(COLOR_BOLD COLOR_YELLOW "  +--------------------------------------------+\n");
    printf("  ¦            MENU DES PRODUITS               ¦\n");
    printf("  +--------------------------------------------+\n" COLOR_RESET);
    printf("\n");
    
    for (i = 0; i < NB_PRODUITS; i++) {
        printf(COLOR_WHITE "  +-----------------------------------------+\n");
        printf("  ¦  " COLOR_BOLD COLOR_GREEN "[%d]" COLOR_RESET "  %-30s    ¦\n", 
               machine.produits[i].id, machine.produits[i].nom);
        printf("  ¦  " COLOR_BOLD COLOR_MAGENTA "Prix : %.2f EUR" COLOR_RESET "                        ¦\n", 
               machine.produits[i].prix);
        printf("  +-----------------------------------------+\n");
        if (i < NB_PRODUITS - 1) printf("\n");
    }
    
    printf("\n");
    printf(COLOR_BOLD COLOR_CYAN "  +--------------------------------------------+\n");
    printf("  ¦  [0]  ??  QUITTER LE PROGRAMME             ¦\n");
    printf("  ¦  [999] ??  ESPACE ADMINISTRATION           ¦\n");
    printf("  +--------------------------------------------+\n" COLOR_RESET);
    printf("\n");
}

/* ============================================================
 *  CYCLE DE VENTE
 * ============================================================ */

void get_date_heure(char *buffer, int taille) {
    time_t t;
    struct tm *tm_info;
    
    time(&t);
    tm_info = localtime(&t);
    strftime(buffer, taille, "%d/%m/%Y %H:%M:%S", tm_info);
}

void afficher_animation(const char *texte, int duree_ms) {
    printf(COLOR_MAGENTA "  %s" COLOR_RESET, texte);
    fflush(stdout);
    {
        int i;
        for (i = 0; i < 3; i++) {
            printf(".");
            fflush(stdout);
            #ifdef _WIN32
                Sleep(duree_ms);
            #else
                usleep(duree_ms * 1000);
            #endif
        }
    }
    printf(COLOR_GREEN " ?\n" COLOR_RESET);
}

void afficher_transaction(Transaction t) {
    printf(COLOR_CYAN "  +--------------------------------------------+\n");
    printf("  ¦           RÉCAPITULATIF DE VOTRE ACHAT     ¦\n");
    printf("  +--------------------------------------------+\n" COLOR_RESET);
    printf("\n");
    printf(COLOR_GREEN "  ? Produit : %s\n", t.produit);
    printf("  ? Prix : %.2f EUR\n", t.prix);
    printf("  ? Montant inséré : %.2f EUR\n", t.montant_insere);
    if (t.monnaie_rendue > 0)
        printf("  ? Monnaie rendue : %.2f EUR\n", t.monnaie_rendue);
    printf("  ? Date/Heure : %s\n" COLOR_RESET, t.date_heure);
    printf("\n");
    printf(COLOR_BOLD COLOR_YELLOW "  ?? MERCI POUR VOTRE ACHAT ! ??\n" COLOR_RESET);
    printf("\n");
}

void cycle_vente(int index) {
    Produit *p    = &machine.produits[index];
    double   prix = p->prix;
    double   total_insere = 0.0;
    double   piece;
    int      ok;
    Transaction current_transaction;
    
    nettoyer_ecran();
    printf("\n");
    printf(COLOR_BOLD COLOR_BLUE "  +--------------------------------------------+\n");
    printf("  ¦            PROCESSUS D'ACHAT               ¦\n");
    printf("  +--------------------------------------------+\n" COLOR_RESET);
    printf("\n");
    printf(COLOR_GREEN "  Vous avez choisi : %s\n" COLOR_RESET, p->nom);
    printf(COLOR_YELLOW "  Prix : %.2f EUR\n" COLOR_RESET, prix);
    printf(COLOR_CYAN "  Pièces acceptées : " COLOR_RESET);
    printf(COLOR_GREEN "0.10  0.20  0.50  1.00  2.00 EUR\n" COLOR_RESET);
    printf("\n");
    
    afficher_animation("  Préparation du distributeur", 500);
    printf("\n");
    
    do {
        printf(COLOR_BOLD COLOR_WHITE "  ?? Montant restant : %.2f EUR\n" COLOR_RESET, arrondir(prix - total_insere));
        printf(COLOR_CYAN "  Insérez une pièce : " COLOR_RESET);

        if (scanf("%lf", &piece) != 1) {
            vider_buffer();
            printf(COLOR_RED "  [!] Saisie invalide.\n" COLOR_RESET);
            continue;
        }
        vider_buffer();

        if (!piece_valide(piece)) {
            printf(COLOR_RED "  [!] Pièce non acceptée. Valeurs valides : 0.10, 0.20, 0.50, 1.00, 2.00\n" COLOR_RESET);
            continue;
        }

        total_insere = arrondir(total_insere + piece);
        printf(COLOR_GREEN "  >> Montant inséré : %.2f EUR ?\n" COLOR_RESET, total_insere);

        ok = (total_insere >= prix - 0.001);
    } while (!ok);

    /* Préparation de la boisson */
    printf("\n");
    afficher_animation("  ? Préparation de votre boisson", 800);
    printf("\n");
    
    /* Affichage de la boisson en ASCII art */
    printf(COLOR_BOLD COLOR_YELLOW);
    printf("     +---+\n");
    printf("     ¦¦¦¦¦\n");
    printf("     ¦¦¦¦¦\n");
    printf("     ¦¦¦¦¦\n");
    printf("     ¦¦¦¦¦\n");
    printf("     +---+\n");
    printf("      ¦ ¦\n");
    printf("     /   \\\n");
    printf(COLOR_RESET);
    printf("\n");

    /* Monnaie rendue */
    double monnaie = arrondir(total_insere - prix);
    if (monnaie > 0.001) {
        printf(COLOR_GREEN "  ?? Monnaie rendue : %.2f EUR\n" COLOR_RESET, monnaie);
        afficher_animation("  Distribution de la monnaie", 300);
        printf("\n");
    } else {
        printf(COLOR_GREEN "  ?? Montant exact ! Pas de monnaie à rendre.\n\n" COLOR_RESET);
    }

    afficher_animation("  ?? Distribution de votre boisson", 400);
    printf("\n");
    
    printf(COLOR_BOLD COLOR_GREEN "  ?? Voici votre %s ! Bonne dégustation ! ??\n\n" COLOR_RESET, p->nom);
    
    /* Enregistrement de la transaction */
    strcpy(current_transaction.produit, p->nom);
    current_transaction.prix = prix;
    current_transaction.montant_insere = total_insere;
    current_transaction.monnaie_rendue = monnaie;
    get_date_heure(current_transaction.date_heure, sizeof(current_transaction.date_heure));
    
    if (machine.nb_transactions < MAX_TRANSACTIONS) {
        machine.transactions[machine.nb_transactions] = current_transaction;
        machine.nb_transactions++;
    }
    
    /* Affichage du récapitulatif */
    afficher_transaction(current_transaction);
    
    /* Mise à jour statistiques */
    p->nb_vendus++;
    machine.chiffre_affaires = arrondir(machine.chiffre_affaires + prix);

    sauvegarder_ventes();
    
    printf(COLOR_CYAN "  Appuyez sur Entrée pour continuer..." COLOR_RESET);
    getchar();
    getchar();
}

/* Verifie si une piece est dans la liste des pieces valides */
int piece_valide(double piece) {
    int i;
    
    for (i = 0; i < NB_PIECES; i++) {
        if (fabs(piece - PIECES_VALIDES[i]) < 0.001)
            return 1;
    }
    return 0;
}

/* Arrondi a 2 decimales pour eviter les erreurs flottantes */
double arrondir(double val) {
    return round(val * 100.0) / 100.0;
}

/* ============================================================
 *  ESPACE ADMINISTRATION
 * ============================================================ */

void menu_administration(void) {
    int choix;
    
    nettoyer_ecran();
    printf(COLOR_BOLD COLOR_RED "\n");
    printf("  +--------------------------------------------+\n");
    printf("  ¦         ESPACE ADMINISTRATION              ¦\n");
    printf("  +--------------------------------------------+\n" COLOR_RESET);
    
    /* Premiere utilisation : creer le compte */
    if (!admin_initialise) {
        printf(COLOR_YELLOW "\n  [ADMIN] Premiere connexion detectee.\n" COLOR_RESET);
        creer_compte_admin();
        admin_initialise = 1;
    }

    /* Authentification */
    if (!authentifier_admin()) {
        printf(COLOR_RED "\n  [!] Acces refuse.\n\n" COLOR_RESET);
        printf(COLOR_CYAN "  Appuyez sur Entrée pour continuer..." COLOR_RESET);
        getchar();
        return;
    }

    do {
        nettoyer_ecran();
        printf(COLOR_BOLD COLOR_RED "\n");
        printf("  +--------------------------------------------+\n");
        printf("  ¦         MENU ADMINISTRATION                ¦\n");
        printf("  +--------------------------------------------+\n" COLOR_RESET);
        printf("\n");
        printf(COLOR_CYAN "  1. ?? Etat de la caisse\n");
        printf("  2. ?? Rapport des ventes\n");
        printf("  3. ?? Historique des transactions\n");
        printf("  4. ?? Changer le mot de passe\n");
        printf("  5. ?? Reinitialiser la machine\n");
        printf("  0. ?? Retour au menu client\n" COLOR_RESET);
        printf("\n");
        printf(COLOR_BOLD COLOR_YELLOW "  +--------------------------------------------+\n");
        printf("  ¦  Saisissez votre choix : " COLOR_RESET);
        
        if (scanf("%d", &choix) != 1) {
            vider_buffer();
            printf(COLOR_RED "\n  [!] Saisie invalide.\n" COLOR_RESET);
            printf(COLOR_CYAN "  Appuyez sur Entrée pour continuer..." COLOR_RESET);
            getchar();
            getchar();
            continue;
        }
        vider_buffer();

        switch (choix) {
            case 1: afficher_caisse(); break;
            case 2: afficher_rapport_ventes(); break;
            case 3: afficher_historique(); break;
            case 4: changer_mot_de_passe(); break;
            case 5: reinitialiser_machine(); break;
            case 0: printf(COLOR_GREEN "\n  Retour au menu client.\n" COLOR_RESET); break;
            default: printf(COLOR_RED "\n  [!] Choix invalide.\n" COLOR_RESET);
        }
        
        if (choix != 0 && choix >= 1 && choix <= 5) {
            printf(COLOR_CYAN "\n  Appuyez sur Entrée pour continuer..." COLOR_RESET);
            getchar();
            getchar();
        }
    } while (choix != 0);
}

/* Creation du compte admin (premiere fois) */
void creer_compte_admin(void) {
    char login[LOGIN_MAX];
    char mdp[MDP_MAX];

    printf(COLOR_YELLOW "\n  Création du compte administrateur:\n" COLOR_RESET);
    printf(COLOR_CYAN "  Choisissez un nom d'utilisateur : " COLOR_RESET);
    fgets(login, LOGIN_MAX, stdin);
    login[strcspn(login, "\n")] = '\0';

    printf(COLOR_CYAN "  Choisissez un mot de passe : " COLOR_RESET);
    fgets(mdp, MDP_MAX, stdin);
    mdp[strcspn(mdp, "\n")] = '\0';

    FILE *f = fopen(AUTH_FILE, "w");
    if (f == NULL) {
        printf(COLOR_RED "  [!] Erreur : impossible de creer le fichier d'authentification.\n" COLOR_RESET);
        return;
    }
    fprintf(f, "%s\n%s\n", login, mdp);
    fclose(f);

    printf(COLOR_GREEN "  ? Compte cree avec succes. Bienvenue, %s !\n" COLOR_RESET, login);
}

/* Authentification admin */
int authentifier_admin(void) {
    char login_stocke[LOGIN_MAX];
    char mdp_stocke[MDP_MAX];
    char mdp_saisi[MDP_MAX];
    int tentatives = 3;

    FILE *f = fopen(AUTH_FILE, "r");
    if (f == NULL) {
        printf(COLOR_RED "  [!] Fichier d'authentification introuvable.\n" COLOR_RESET);
        return 0;
    }
    fgets(login_stocke, LOGIN_MAX, f);
    fgets(mdp_stocke,   MDP_MAX,   f);
    fclose(f);

    login_stocke[strcspn(login_stocke, "\n")] = '\0';
    mdp_stocke  [strcspn(mdp_stocke,   "\n")] = '\0';

    do {
        printf(COLOR_CYAN "\n  Mot de passe : " COLOR_RESET);
        fgets(mdp_saisi, MDP_MAX, stdin);
        mdp_saisi[strcspn(mdp_saisi, "\n")] = '\0';

        if (strcmp(mdp_saisi, mdp_stocke) == 0) {
            printf(COLOR_GREEN "  ? Acces autorise. Bonjour, %s !\n" COLOR_RESET, login_stocke);
            return 1;
        }
        tentatives--;
        if (tentatives > 0)
            printf(COLOR_RED "  [!] Mot de passe incorrect. %d tentative(s) restante(s).\n" COLOR_RESET, tentatives);
    } while (tentatives > 0);

    return 0;
}

/* ============================================================
 *  FONCTIONS ADMIN
 * ============================================================ */

void afficher_caisse(void) {
    nettoyer_ecran();
    printf(COLOR_BOLD COLOR_YELLOW "\n");
    printf("  +--------------------------------------------+\n");
    printf("  ¦           ETAT DE LA CAISSE                 ¦\n");
    printf("  +--------------------------------------------+\n" COLOR_RESET);
    printf(COLOR_GREEN "\n  ?? Chiffre d'affaires total : %.2f EUR\n\n" COLOR_RESET, machine.chiffre_affaires);
}

void afficher_rapport_ventes(void) {
    int i;
    
    nettoyer_ecran();
    printf(COLOR_BOLD COLOR_YELLOW "\n");
    printf("  +--------------------------------------------+\n");
    printf("  ¦          RAPPORT DES VENTES                ¦\n");
    printf("  +--------------------------------------------+\n" COLOR_RESET);
    printf("\n");
    printf(COLOR_CYAN "  %-22s  %s\n", "Produit", "Quantite vendue");
    printf("  %-22s  %s\n" COLOR_RESET, "----------------------", "---------------");
    for (i = 0; i < NB_PRODUITS; i++) {
        printf(COLOR_GREEN "  %-22s  %d\n" COLOR_RESET,
               machine.produits[i].nom,
               machine.produits[i].nb_vendus);
    }
    printf("\n");
}

void afficher_historique(void) {
    int i;
    
    nettoyer_ecran();
    printf(COLOR_BOLD COLOR_YELLOW "\n");
    printf("  +--------------------------------------------+\n");
    printf("  ¦       HISTORIQUE DES TRANSACTIONS          ¦\n");
    printf("  +--------------------------------------------+\n" COLOR_RESET);
    
    if (machine.nb_transactions == 0) {
        printf(COLOR_RED "\n  Aucune transaction effectuee pour le moment.\n\n" COLOR_RESET);
        return;
    }
    
    printf("\n");
    for (i = 0; i < machine.nb_transactions; i++) {
        printf(COLOR_CYAN "  +--------------------------------------------+\n");
        printf("  ¦  Transaction #%d                              ¦\n", i+1);
        printf("  +--------------------------------------------+\n" COLOR_RESET);
        printf(COLOR_GREEN "  Produit : %s\n", machine.transactions[i].produit);
        printf("  Prix : %.2f EUR\n", machine.transactions[i].prix);
        printf("  Montant insere : %.2f EUR\n", machine.transactions[i].montant_insere);
        printf("  Monnaie rendue : %.2f EUR\n", machine.transactions[i].monnaie_rendue);
        printf("  Date/Heure : %s\n" COLOR_RESET, machine.transactions[i].date_heure);
        printf("\n");
    }
}

void changer_mot_de_passe(void) {
    char login_stocke[LOGIN_MAX];
    char mdp_ancien[MDP_MAX];
    char mdp_nouveau[MDP_MAX];

    /* Lire login actuel */
    FILE *f = fopen(AUTH_FILE, "r");
    if (f == NULL) {
        printf(COLOR_RED "  [!] Fichier introuvable.\n" COLOR_RESET);
        return;
    }
    fgets(login_stocke, LOGIN_MAX, f);
    fgets(mdp_ancien,   MDP_MAX,   f);
    fclose(f);
    login_stocke[strcspn(login_stocke, "\n")] = '\0';
    mdp_ancien  [strcspn(mdp_ancien,   "\n")] = '\0';

    printf(COLOR_YELLOW "\n  Changement du mot de passe:\n" COLOR_RESET);
    printf(COLOR_CYAN "  Nouveau mot de passe : " COLOR_RESET);
    fgets(mdp_nouveau, MDP_MAX, stdin);
    mdp_nouveau[strcspn(mdp_nouveau, "\n")] = '\0';

    if (strlen(mdp_nouveau) == 0) {
        printf(COLOR_RED "  [!] Le mot de passe ne peut pas etre vide.\n" COLOR_RESET);
        return;
    }

    f = fopen(AUTH_FILE, "w");
    if (f == NULL) {
        printf(COLOR_RED "  [!] Erreur d'ecriture.\n" COLOR_RESET);
        return;
    }
    fprintf(f, "%s\n%s\n", login_stocke, mdp_nouveau);
    fclose(f);

    printf(COLOR_GREEN "  ? Mot de passe mis a jour avec succes.\n" COLOR_RESET);
}

void reinitialiser_machine(void) {
    int i;
    char confirm[10];
    
    printf(COLOR_RED "\n  ??  ATTENTION : Cette action supprimera toutes les donnees !\n" COLOR_RESET);
    printf(COLOR_YELLOW "  Confirmer la reinitialisation ? (oui/non) : " COLOR_RESET);
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = '\0';

    if (strcmp(confirm, "oui") != 0) {
        printf(COLOR_GREEN "  Reinitialisation annulee.\n" COLOR_RESET);
        return;
    }

    for (i = 0; i < NB_PRODUITS; i++)
        machine.produits[i].nb_vendus = 0;
    machine.chiffre_affaires = 0.0;
    machine.nb_transactions = 0;

    /* Effacer le fichier de ventes */
    FILE *f = fopen(VENTE_FILE, "wb");
    if (f) fclose(f);

    printf(COLOR_GREEN "  ? Machine reinitialisee avec succes.\n" COLOR_RESET);
}

/* ============================================================
 *  PERSISTANCE (FICHIERS)
 * ============================================================ */

void sauvegarder_ventes(void) {
    int i;
    FILE *f = fopen(VENTE_FILE, "wb");
    
    if (f == NULL) {
        printf(COLOR_RED "  [!] Erreur : impossible de sauvegarder les ventes.\n" COLOR_RESET);
        return;
    }
    
    fwrite(&machine.chiffre_affaires, sizeof(double), 1, f);
    for (i = 0; i < NB_PRODUITS; i++)
        fwrite(&machine.produits[i].nb_vendus, sizeof(int), 1, f);
    
    fwrite(&machine.nb_transactions, sizeof(int), 1, f);
    for (i = 0; i < machine.nb_transactions; i++) {
        fwrite(&machine.transactions[i], sizeof(Transaction), 1, f);
    }
    
    fclose(f);
}

void charger_ventes(void) {
    int i;
    FILE *f = fopen(VENTE_FILE, "rb");
    
    if (f == NULL) return;

    fread(&machine.chiffre_affaires, sizeof(double), 1, f);
    for (i = 0; i < NB_PRODUITS; i++)
        fread(&machine.produits[i].nb_vendus, sizeof(int), 1, f);
    
    fread(&machine.nb_transactions, sizeof(int), 1, f);
    for (i = 0; i < machine.nb_transactions && i < MAX_TRANSACTIONS; i++) {
        fread(&machine.transactions[i], sizeof(Transaction), 1, f);
    }
    
    fclose(f);
}

/* ============================================================
 *  UTILITAIRES
 * ============================================================ */

/* Vide le buffer stdin pour eviter les lectures parasites */
void vider_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void nettoyer_ecran(void) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
