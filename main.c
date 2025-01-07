#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LARGURA_DEFAULT 2
#define ALTURA_DEFAULT 50
#define ESPACAMENTO_DEFAULT 4
#define LARGURA_TOTAL 209  // Largura total com bordas
#define ALTURA_TOTAL 58    // Altura total com bordas
#define L_CODE_SIZE 7
#define R_CODE_SIZE 7

// Tabelas de codificação L-code e R-code para EAN-8
const char* l_code[] = {
    "0001101", "0011001", "0010011", "0111101", "0100011",
    "0110001", "0101111", "0111011", "0110111", "0001011"
};

const char* r_code[] = {
    "1110010", "1100110", "1101100", "1000010", "1011100",
    "1001110", "1010000", "1000100", "1001000", "1110100"
};

int areas_preenchidas[LARGURA_TOTAL];
int indice_pbm = 0;

// Função para verificar se o identificador é válido
int is_valid_identifier(const char *identifier) {
    if (strlen(identifier) != 8) {
        return 0;
    }
    for (int i = 0; i < 8; i++) {
        if (!isdigit(identifier[i])) {
            return 0;
        }
    }

    // Calculando o dígito verificador
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += (identifier[i] - '0') * (i % 2 == 0 ? 3 : 1);
    }
    int check_digit = (10 - (sum % 10)) % 10;

    return check_digit == (identifier[7] - '0');
}

// Função para gerar a imagem PBM
void gerar_pbm(const char *codigo, const char *nome_arquivo, int espacamento, int largura, int altura) {
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo para escrita.\n");
        return;
    }

    int largura_total = largura * 100 + espacamento * 2;
    int altura_total = altura + espacamento * 2;

    // Cabeçalho do arquivo PBM
    fprintf(arquivo, "P1\n%d %d\n", largura_total, altura_total);

    // Gerar as linhas do código de barras
    for (int y = 0; y < altura_total; y++) {
        for (int x = 0; x < largura_total; x++) {
            if (y < espacamento || y >= altura_total - espacamento ||
                x < espacamento || x >= largura_total - espacamento) {
                fprintf(arquivo, "0 "); // Espaços de borda
            } else {
                int posicao = (x - espacamento) / largura;
                int bit = 0;

                // Código de barras começa com o padrão de start (3 barras)
                if (posicao < 3) {
                    bit = (posicao == 0 || posicao == 2);
                }
                // Lado esquerdo do código (L-code)
                else if (posicao >= 3 && posicao < 31) {
                    int indice = (posicao - 3) / L_CODE_SIZE;
                    int deslocamento = (posicao - 3) % L_CODE_SIZE;
                    bit = l_code[codigo[indice] - '0'][deslocamento] - '0';
                }
                // Meio do código de barras
                else if (posicao >= 31 && posicao < 34) {
                    bit = (posicao % 2 == 0); // Espaços do meio
                }
                // Lado direito do código (R-code)
                else if (posicao >= 34 && posicao < 62) {
                    int indice = (posicao - 34) / R_CODE_SIZE;
                    int deslocamento = (posicao - 34) % R_CODE_SIZE;
                    bit = r_code[codigo[indice + 4] - '0'][deslocamento] - '0';
                }
                // Final do código de barras
                else if (posicao >= 62) {
                    bit = (posicao % 2 == 0); // Final com 3 barras
                }

                fprintf(arquivo, "%d ", bit);
                if (y == espacamento) {
                    areas_preenchidas[indice_pbm++] = bit;
                }
            }
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
    printf("\nArquivo PBM gerado: %s\n", nome_arquivo);
    printf("\nValores da linha %d armazenados: \n", espacamento);
    for (int i = 0; i < indice_pbm; i++) {
        printf("%d ", areas_preenchidas[i]);
    }
    printf("\n");
}

// Função para exibir o menu
void exibir_menu() {
    printf("\n===================================\n");
    printf("   Gerador de Código de Barras EAN-8   \n");
    printf("===================================\n");
    printf("1. Gerar código de barras\n");
    printf("2. Sair\n");
    printf("===================================\n");
    printf("Escolha uma opção: ");
}

int main() {
    int opcao;

    do {
        exibir_menu();
        scanf("%d", &opcao);
        getchar();

        switch (opcao) {
            case 1: {
                char codigo[9]; // Identificador do código de barras (8 dígitos + '\0')
                char nome_arquivo[256] = "codigo_barras.pbm"; // Nome padrão do arquivo
                int espacamento = ESPACAMENTO_DEFAULT;
                int largura = LARGURA_DEFAULT;
                int altura = ALTURA_DEFAULT;

                printf("Digite o identificador do código de barras (8 dígitos): ");
                scanf("%8s", codigo);

                if (!is_valid_identifier(codigo)) {
                    printf("Erro: identificador inválido.\n");
                    break;
                }

                // Ajustar espaçamento, largura e altura se necessário
                printf("Digite o espaçamento lateral (Enter para padrão %d): ", ESPACAMENTO_DEFAULT);
                if (scanf("%d", &espacamento) != 1) espacamento = ESPACAMENTO_DEFAULT;
                getchar();

                printf("Digite a largura de cada área (Enter para padrão %d): ", LARGURA_DEFAULT);
                if (scanf("%d", &largura) != 1) largura = LARGURA_DEFAULT;
                getchar();

                printf("Digite a altura do código (Enter para padrão %d): ", ALTURA_DEFAULT);
                if (scanf("%d", &altura) != 1) altura = ALTURA_DEFAULT;
                getchar();

                // Gerar o código de barras
                gerar_pbm(codigo, nome_arquivo, espacamento, largura, altura);
                break;
            }
            case 2:
                printf("Saindo do programa...\n");
                break;
            default:
                printf("Opção inválida! Tente novamente.\n");
        }
    } while (opcao != 2);

    return 0;
}
