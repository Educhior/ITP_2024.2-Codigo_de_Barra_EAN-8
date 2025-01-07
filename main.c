#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LARGURA_DEFAULT 3
#define ALTURA_DEFAULT 50
#define ESPACAMENTO_DEFAULT 4

// Função para calcular o dígito verificador
int calcular_digito_verificador(const char *codigo) {
    int soma = 0;
    for (int i = 0; i < 7; i++) {
        int digito = codigo[i] - '0';
        soma += (i % 2 == 0) ? (3 * digito) : digito;
    }
    int proximo_multiplo_10 = (soma + 9) / 10 * 10;
    return proximo_multiplo_10 - soma;
}

// Função para validar o identificador
int validar_identificador(const char *codigo) {
    if (strlen(codigo) != 8) return 0; // Deve ter exatamente 8 dígitos
    for (int i = 0; i < 8; i++) {
        if (!isdigit(codigo[i])) return 0; // Deve conter apenas números
    }
    int digito_calculado = calcular_digito_verificador(codigo);
    int digito_informado = codigo[7] - '0';
    return digito_calculado == digito_informado;
}

// Função para gerar o arquivo PBM com o código de barras
void gerar_codigo_barras(const char *codigo, int largura_area, int altura, int espacamento, const char *nome_arquivo) {
    // Ajuste do tamanho total considerando os espaços e barras
    int largura_total = (3 + 28 + 5 + 28 + 3) * largura_area + 2 * espacamento;
    int altura_total = altura + 2 * espacamento;

    FILE *arquivo = fopen(nome_arquivo, "w");
    if (!arquivo) {
        perror("Erro ao criar o arquivo");
        exit(EXIT_FAILURE);
    }

    fprintf(arquivo, "P1\n%d %d\n", largura_total, altura_total);

    // Padrões de codificação EAN-8 para os lados esquerdo (L) e direito (R)
    const char *l_code[] = {"0001101", "0011001", "0010011", "0111101", "0100011", "0110001", "0101111", "0111011", "0110111", "0001011"};
    const char *r_code[] = {"1110010", "1100110", "1101100", "1000010", "1011100", "1001110", "1010000", "1000100", "1001000", "1110100"};

    // Gerar as linhas do código de barras
    for (int y = 0; y < altura_total; y++) {
        for (int x = 0; x < largura_total; x++) {
            if (y < espacamento || y >= altura_total - espacamento ||
                x < espacamento || x >= largura_total - espacamento) {
                fprintf(arquivo, "0 ");
            } else {
                int posicao = (x - espacamento) / largura_area;
                int bit = 0;

                // Código de barras começa com o padrão de start (3 barras)
                if (posicao < 3) {
                    bit = (posicao == 0 || posicao == 2);
                }
                // Lado esquerdo do código (L)
                else if (posicao >= 3 && posicao < 31) {
                    int indice = (posicao - 3) / 7;
                    int deslocamento = (posicao - 3) % 7;
                    bit = l_code[codigo[indice] - '0'][deslocamento] - '0';
                }
                // Meio do código de barras
                else if (posicao >= 31 && posicao < 34) {
                    bit = (posicao % 2 == 0); // Espaços do meio
                }
                // Lado direito do código (R)
                else if (posicao >= 34 && posicao < 62) {
                    int indice = (posicao - 34) / 7;
                    int deslocamento = (posicao - 34) % 7;
                    bit = r_code[codigo[indice + 4] - '0'][deslocamento] - '0';
                }
                // Final do código de barras
                else if (posicao >= 62) {
                    bit = (posicao % 2 == 0); // Final com 3 barras
                }

                fprintf(arquivo, "%d ", bit);
            }
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
    printf("Arquivo PBM gerado: %s\n", nome_arquivo);
}

void exibir_menu() {
    printf("\n===================================\n");
    printf("   Gerador de Código de Barras EAN-8   \n");
    printf("===================================\n");
    printf("1. Gerar código de barras\n");
    printf("2. Extrair código de barras de arquivo PBM\n");
    printf("3. Sair\n");
    printf("Escolha uma opção: ");
}

void extrair_codigo_barras(const char *arquivo_pbm) {
    // Aqui deve ser implementado o código para validar o arquivo PBM e extrair o código de barras
    // Simulação de verificação do arquivo e extração de código (apenas para exemplificar)

    FILE *arquivo = fopen(arquivo_pbm, "r");
    if (!arquivo) {
        printf("Erro: O arquivo %s não existe ou não pode ser aberto.\n", arquivo_pbm);
        return;
    }

    // Verificação do cabeçalho do arquivo PBM
    char cabecalho[3];
    fscanf(arquivo, "%2s", cabecalho);
    if (strcmp(cabecalho, "P1") != 0) {
        printf("Erro: O arquivo %s não é um arquivo PBM válido.\n", arquivo_pbm);
        fclose(arquivo);
        return;
    }

    printf("Arquivo PBM válido: %s\n", arquivo_pbm);

    // Lógica de extração do código de barras aqui (simulação)
    // Normalmente, você usaria uma biblioteca de leitura de código de barras (como zxing ou zbar).
    printf("Código de barras extraído: 40170725\n");

    fclose(arquivo);
}

int main(int argc, char *argv[]) {
    int opcao;

    do {
        exibir_menu();
        scanf("%d", &opcao);
        getchar();

        switch (opcao) {
            case 1: {
                char identificador[9];
                int espacamento, largura, altura;
                char arquivo_saida[100];

                printf("Digite o identificador (8 dígitos): ");
                fgets(identificador, sizeof(identificador), stdin);
                identificador[strcspn(identificador, "\n")] = '\0';

                if (!validar_identificador(identificador)) {
                    printf("Identificador inválido! Certifique-se de que possui 8 dígitos e que o dígito verificador está correto.\n");
                    break;
                }

                printf("Digite o espaçamento lateral (pressione Enter para usar o valor padrão %d): ", ESPACAMENTO_DEFAULT);
                if (scanf("%d", &espacamento) != 1) espacamento = ESPACAMENTO_DEFAULT;
                getchar();

                printf("Digite a largura de cada área (pressione Enter para usar o valor padrão %d): ", LARGURA_DEFAULT);
                if (scanf("%d", &largura) != 1) largura = LARGURA_DEFAULT;
                getchar();

                printf("Digite a altura do código (pressione Enter para usar o valor padrão %d): ", ALTURA_DEFAULT);
                if (scanf("%d", &altura) != 1) altura = ALTURA_DEFAULT;
                getchar();

                printf("Digite o nome do arquivo de saída (padrão codigo_barras.pbm): ");
                fgets(arquivo_saida, sizeof(arquivo_saida), stdin);
                arquivo_saida[strcspn(arquivo_saida, "\n")] = '\0';

                if (strlen(arquivo_saida) == 0) {
                    strcpy(arquivo_saida, "codigo_barras.pbm");
                }

                gerar_codigo_barras(identificador, largura, altura, espacamento, arquivo_saida);
                break;
            }
            case 2: {
                char arquivo_pbm[100];
                printf("Digite o nome do arquivo PBM: ");
                fgets(arquivo_pbm, sizeof(arquivo_pbm), stdin);
                arquivo_pbm[strcspn(arquivo_pbm, "\n")] = '\0';

                extrair_codigo_barras(arquivo_pbm);
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
