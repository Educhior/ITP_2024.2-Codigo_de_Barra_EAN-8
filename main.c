#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LARGURA_DEFAULT 3
#define ALTURA_DEFAULT 50
#define ESPACAMENTO_DEFAULT 4

// Funções auxiliares para manipulação de PBM
int ler_cabecalho_pbm(FILE *arquivo, int *largura, int *altura) {
    char buffer[256]; // Buffer maior para lidar com comentários ou linhas longas

    // Verificar a assinatura "P1"
    if (!fgets(buffer, sizeof(buffer), arquivo) || strncmp(buffer, "P1", 2) != 0) {
        fprintf(stderr, "Erro: Assinatura do arquivo PBM invalida ou ausente.\n");
        return 0;
    }

    // Ignorar comentários e espaços em branco
    do {
        if (!fgets(buffer, sizeof(buffer), arquivo)) {
            fprintf(stderr, "Erro: Fim inesperado do arquivo ao ler dimensoes.\n");
            return 0;
        }
    } while (buffer[0] == '#' || isspace(buffer[0]));

    // Ler largura e altura
    if (sscanf(buffer, "%d %d", largura, altura) != 2) {
        fprintf(stderr, "Erro: Dimensoes do arquivo PBM invalidas.\n");
        return 0;
    }

    // Validar dimensões
    if (*largura <= 0 || *altura <= 0) {
        fprintf(stderr, "Erro: Dimensoes do arquivo PBM devem ser maiores que zero.\n");
        return 0;
    }

    return 1; // Cabeçalho válido
}


// Função para extrair o identificador do código de barras
void extrair_codigo_barras(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        fprintf(stderr, "Erro: O arquivo '%s' não foi encontrado.\n", nome_arquivo);
        exit(EXIT_FAILURE);
    }

    int largura, altura;
    if (!ler_cabecalho_pbm(arquivo, &largura, &altura)) {
        fclose(arquivo);
        fprintf(stderr, "Erro: O arquivo '%s' não é um arquivo PBM válido.\n", nome_arquivo);
        exit(EXIT_FAILURE);
    }

    // Alocar memória para a matriz de imagem
    int **imagem = (int **)malloc(altura * sizeof(int *));
    if (!imagem) {
        fprintf(stderr, "Erro: Falha ao alocar memória para a imagem.\n");
        fclose(arquivo);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < altura; i++) {
        imagem[i] = (int *)malloc(largura * sizeof(int));
        if (!imagem[i]) {
            fprintf(stderr, "Erro: Falha ao alocar memória para a linha %d.\n", i);
            for (int j = 0; j < i; j++) free(imagem[j]);
            free(imagem);
            fclose(arquivo);
            exit(EXIT_FAILURE);
        }
    }

    // Ler os dados do arquivo PBM
    for (int i = 0; i < altura; i++) {
        for (int j = 0; j < largura; j++) {
            if (fscanf(arquivo, "%d", &imagem[i][j]) != 1) {
                fprintf(stderr, "Erro: Dados do arquivo PBM estão corrompidos ou incompletos.\n");
                for (int k = 0; k < altura; k++) free(imagem[k]);
                free(imagem);
                fclose(arquivo);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(arquivo);

    // Processar a imagem para encontrar o código de barras
    int largura_barras = LARGURA_DEFAULT; // Largura esperada de cada barra
    int inicio = ESPACAMENTO_DEFAULT;    // Ignorar espaçamento inicial

    char identificador[9] = {0};
    int indice = 0;

    for (int x = inicio; x < largura - inicio && indice < 8; x += largura_barras * 7) {
        int digito = 0;
        for (int i = 0; i < 7; i++) {
            if (x + i >= largura) { // Verificação de limite
                fprintf(stderr, "Erro: Código de barras malformado na imagem.\n");
                for (int i = 0; i < altura; i++) free(imagem[i]);
                free(imagem);
                exit(EXIT_FAILURE);
            }
            digito = digito << 1 | imagem[altura / 2][x + i];
        }

        // Debug: Mostrar os valores de cada "barra"
        printf("Valor do digito: %d (Binário: ", digito);
        for (int i = 0; i < 7; i++) {
            printf("%d", (imagem[altura / 2][x + i]));
        }
        printf(")\n");

        // Verifique se o dígito está dentro do intervalo válido
        if (digito < 0 || digito > 9) {
            fprintf(stderr, "Erro: Digito inválido detectado: %d\n", digito);
            for (int i = 0; i < altura; i++) free(imagem[i]);
            free(imagem);
            exit(EXIT_FAILURE);
        }

        // Decodificar o dígito com base nos padrões de barras
        const char *l_code[] = {"0001101", "0011001", "0010011", "0111101", "0100011", "0110001", "0101111", "0111011", "0110111", "0001011"};
        const char *r_code[] = {"1110010", "1100110", "1101100", "1000010", "1011100", "1001110", "1010000", "1000100", "1001000", "1110100"};

        char digito_str[8];
        sprintf(digito_str, "%07d", digito);

        int encontrado = 0;
        for (int d = 0; d < 10; d++) {
            if (strcmp(l_code[d], digito_str) == 0 || strcmp(r_code[d], digito_str) == 0) {
                identificador[indice++] = '0' + d;
                encontrado = 1;
                break;
            }
        }

        if (!encontrado) {
            fprintf(stderr, "Erro: Código de barras inválido na imagem.\n");
            for (int i = 0; i < altura; i++) free(imagem[i]);
            free(imagem);
            exit(EXIT_FAILURE);
        }
    }

    if (indice != 8) {
        fprintf(stderr, "Erro: Código de barras não encontrado na imagem.\n");
        for (int i = 0; i < altura; i++) free(imagem[i]);
        free(imagem);
        exit(EXIT_FAILURE);
    }

    // Exibir o identificador encontrado
    printf("Identificador encontrado: %s\n", identificador);

    // Liberar memória da matriz de imagem
    for (int i = 0; i < altura; i++) free(imagem[i]);
    free(imagem);
}


// Exibição de menu e funcionalidade de geração já implementada

void exibir_menu() {
    printf("\n===================================\n");
    printf("   Gerador e Extrator de Codigo de Barras EAN-8\n");
    printf("===================================\n");
    printf("1. Gerar codigo de barras\n");
    printf("2. Extrair codigo de barras de imagem PBM\n");
    printf("3. Sair\n");
    printf("Escolha uma opcao: ");
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        // Extrair identificador diretamente do arquivo PBM
        extrair_codigo_barras(argv[1]);
        return 0;
    }

    int opcao;
    do {
        exibir_menu();

        if (scanf("%d", &opcao) != 1) {
            printf("Opção inválida! Tente novamente.\n");
            while (getchar() != '\n');
            continue;
        }

        while (getchar() != '\n');

        switch (opcao) {
            case 1:
                // Função de geração de código de barras aqui
                printf("Funcionalidade de geração de código de barras.\n");
                break;
            case 2: {
                char arquivo[100];
                printf("Digite o nome do arquivo PBM: ");
                fgets(arquivo, sizeof(arquivo), stdin);
                arquivo[strcspn(arquivo, "\n")] = '\0'; // Remover newline
                extrair_codigo_barras(arquivo);
                break;
            }
            case 3:
                printf("Saindo do programa...\n");
                break;
            default:
                printf("Opção inválida! Tente novamente.\n");
        }
    } while (opcao != 3);

    return 0;
}
