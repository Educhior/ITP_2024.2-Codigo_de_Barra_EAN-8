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
    int largura_total = (3 + 28 + 5 + 28 + 3) * largura_area + 2 * espacamento;
    int altura_total = altura + 2 * espacamento;

    FILE *arquivo = fopen(nome_arquivo, "w");
    if (!arquivo) {
        perror("Erro ao criar o arquivo");
        exit(EXIT_FAILURE);
    }

    fprintf(arquivo, "P1\n%d %d\n", largura_total, altura_total);

    const char *l_code[] = {"0001101", "0011001", "0010011", "0111101", "0100011", "0110001", "0101111", "0111011", "0110111", "0001011"};
    const char *r_code[] = {"1110010", "1100110", "1101100", "1000010", "1011100", "1001110", "1010000", "1000100", "1001000", "1110100"};

    for (int y = 0; y < altura_total; y++) {
        for (int x = 0; x < largura_total; x++) {
            if (y < espacamento || y >= altura_total - espacamento ||
                x < espacamento || x >= largura_total - espacamento) {
                fprintf(arquivo, "0 ");
            } else {
                int posicao = (x - espacamento) / largura_area;
                int bit = 0;

                if (posicao < 3) {
                    bit = (posicao == 0 || posicao == 2);
                } else if (posicao >= 3 && posicao < 31) {
                    int indice = (posicao - 3) / 7;
                    int deslocamento = (posicao - 3) % 7;
                    bit = l_code[codigo[indice] - '0'][deslocamento] - '0';
                } else if (posicao >= 31 && posicao < 34) {
                    bit = (posicao % 2 == 0);
                } else if (posicao >= 34 && posicao < 62) {
                    int indice = (posicao - 34) / 7;
                    int deslocamento = (posicao - 34) % 7;
                    bit = r_code[codigo[indice + 4] - '0'][deslocamento] - '0';
                } else if (posicao >= 62) {
                    bit = (posicao % 2 == 0);
                }

                fprintf(arquivo, "%d ", bit);
            }
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
    printf("Arquivo PBM gerado: %s\n", nome_arquivo);
}

// Função para ler identificador e validar
void ler_identificador(char *identificador) {
    while (1) {
        printf("Digite o identificador (8 digitos): ");
        fgets(identificador, 9, stdin);
        identificador[strcspn(identificador, "\n")] = '\0'; // Remover nova linha

        if (strlen(identificador) != 8 || !validar_identificador(identificador)) {
            printf("Identificador invalido! Certifique-se de que possui 8 digitos e que o digito verificador esta correto.\n");
        } else {
            break;
        }
    }
}

// Função para ler um número e aplicar um valor padrão se a entrada for vazia
int ler_int_com_padrao(const char *mensagem, int valor_padrao) {
    char buffer[10];
    printf("%s (padrao %d): ", mensagem, valor_padrao);
    fgets(buffer, sizeof(buffer), stdin);
    if (buffer[0] == '\n') {
        return valor_padrao; // Retorna o valor padrão caso a entrada seja vazia
    }
    return atoi(buffer); // Converte a entrada em inteiro
}

// Função para ler o nome do arquivo e aplicar um valor padrão se a entrada for vazia
void ler_arquivo_saida(char *arquivo_saida, const char *valor_padrao) {
    printf("Digite o nome do arquivo de saida (padrao %s): ", valor_padrao);
    fgets(arquivo_saida, 100, stdin);
    arquivo_saida[strcspn(arquivo_saida, "\n")] = '\0'; // Remover nova linha
    if (strlen(arquivo_saida) == 0) {
        strcpy(arquivo_saida, valor_padrao); // Usa o valor padrão se não for fornecido nome de arquivo
    }
}

// Função para exibir o menu
void exibir_menu() {
    printf("\n===================================\n");
    printf("   Gerador de Codigo de Barras EAN-8   \n");
    printf("===================================\n");
    printf("1. Gerar codigo de barras\n");
    printf("2. Sair\n");
    printf("Escolha uma opcao: ");
}

int main() {
    int opcao;
    do {
        exibir_menu();  // Exibe o menu sempre

        // Espera pela entrada do usuário para selecionar uma opção
        if (scanf("%d", &opcao) != 1) {
            printf("Opção inválida! Tente novamente.\n");
            // Limpar buffer para o caso de entradas erradas
            while (getchar() != '\n');
            continue;
        }

        // Limpar o buffer do teclado
        while (getchar() != '\n');

        switch (opcao) {
            case 1: {
                char identificador[9];
                int espacamento = ler_int_com_padrao("Digite o espacamento lateral", ESPACAMENTO_DEFAULT);
                int largura = ler_int_com_padrao("Digite a largura de cada area", LARGURA_DEFAULT);
                int altura = ler_int_com_padrao("Digite a altura do codigo", ALTURA_DEFAULT);
                char arquivo_saida[100];
                ler_arquivo_saida(arquivo_saida, "codigo_barras.pbm");

                // Solicita o identificador (8 dígitos)
                ler_identificador(identificador);

                gerar_codigo_barras(identificador, largura, altura, espacamento, arquivo_saida);
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
