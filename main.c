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
    printf("   Gerador de Codigo de Barras EAN-8   \n");
    printf("===================================\n");
    printf("1. Gerar codigo de barras\n");
    printf("2. Sair\n");
    printf("Escolha uma opcao: ");
}

int obter_entrada_inteira(const char *mensagem, int valor_default) {
    int valor;
    char buffer[100];

    printf("%s", mensagem);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] == '\n') {
            return valor_default;
        }
        if (sscanf(buffer, "%d", &valor) == 1) {
            return valor;
        }
    }
    return valor_default;
}

// Função para limpar o buffer e esperar o "Enter" após uma entrada
void limpar_buffer() {
    while (getchar() != '\n'); // Limpar o buffer
}

int main() {
    int opcao;
    do {
        exibir_menu();
        scanf("%d", &opcao);
        limpar_buffer(); // Limpar o buffer após a opção

        switch (opcao) {
            case 1: {
                char identificador[9];
                int espacamento, largura, altura;
                char arquivo_saida[100];

                printf("Digite o identificador (8 digitos): ");
                fgets(identificador, sizeof(identificador), stdin);
                identificador[strcspn(identificador, "\n")] = '\0'; // Remover o '\n' do buffer

                // Verifica se a entrada foi vazia ou inválida
                if (strlen(identificador) != 8 || !validar_identificador(identificador)) {
                    printf("Identificador invalido! Certifique-se de que possui 8 digitos e que o digito verificador esta correto.\n");
                    break;
                }

                espacamento = obter_entrada_inteira("Digite o espacamento lateral (pressione Enter para usar o valor padrao 4): ", ESPACAMENTO_DEFAULT);
                largura = obter_entrada_inteira("Digite a largura de cada area (pressione Enter para usar o valor padrao 3): ", LARGURA_DEFAULT);
                altura = obter_entrada_inteira("Digite a altura do codigo (pressione Enter para usar o valor padrao 50): ", ALTURA_DEFAULT);

                printf("Digite o nome do arquivo de saida (pressione Enter para usar o valor padrao codigo_barras.pbm): ");
                fgets(arquivo_saida, sizeof(arquivo_saida), stdin);
                arquivo_saida[strcspn(arquivo_saida, "\n")] = '\0';

                if (strlen(arquivo_saida) == 0) {
                    strcpy(arquivo_saida, "codigo_barras.pbm");
                }

                gerar_codigo_barras(identificador, largura, altura, espacamento, arquivo_saida);
                break;
            }
            case 2:
                printf("Saindo do programa...\n");
                break;
            default:
                printf("Opcao invalida! Tente novamente.\n");
        }
    } while (opcao != 2);

    return 0;
}
