---
## USER 🧑‍💻

não altere nenhum arquivo, seu foco é diagnóstico
veja as ferramentas mcp em tools/
identifique oportunidade DRY e SRP
sobre o SRP, é no sentido de ser o mesmo objeto, classe, método, dicionário ou função não terem várias responsabilidades, mas não o mesmo arquivo
no mesmo arquivo, varias responsabilidades do mesmo assunto tudo bem

---
## MODEL ✨

### Oportunidades Identificadas (Diagnóstico)

#### 1. DRY (Don't Repeat Yourself)
*   **Lógica de Regras Genéricas:** O `cpp_verifier` e o `python_verifier` ambos implementam `_return_spacing`, embora com regras ligeiramente diferentes. A estrutura de chamar `self._apply( rule[0], rule[1], rule[2] )` é repetitiva.
*   **Oportunidade:** A classe `base_verifier` poderia ter um método `run_rules()` que itera sobre um dicionário de regras configurado nas subclasses, automatizando a execução de `_apply` para padrões simples de regex sem precisar sobrescrever `run()` para cada pequena adição.
*   **Configuração de Comentários:** O `cpp_verifier` busca o caractere de comentário em `cpp_config.py`, enquanto o `python_verifier` (herdando da base) usa o valor default. Isso poderia ser centralizado em um sistema de configuração mais uniforme.
    -   Veredito: a estrutura de linguagens devem ser o mais simétrica possível. Falta um `default_python_config` assim como já existe um `default_cpp_config`

#### 2. SRP (Single Responsibility Principle)
*   **`run_verifier` em `base_verifier.py`:** Este método lida com:
    1.  Validação de parâmetros de entrada.
    2.  Iteração sobre arquivos.
    3.  Leitura/Escrita de arquivos.
    4.  Validação de extensões.
    5.  Formatação da mensagem de saída final.
*   **Oportunidade:** O `run_verifier` está fazendo muita coisa. A responsabilidade de "orquestração de arquivos" poderia ser separada da "geração de relatório". Além disso, a lógica de "se mudou, escreve" poderia estar em um método dedicado da `base_verifier` ou em uma classe utilitária de IO.
*   **`cpp_verifier.py` e `_bracket_spacing`:** O método `_bracket_spacing` é complexo e contém lógica de "ignorar strings/comentários" misturada com a aplicação das regras. Esta lógica de "regex com exclusão" é útil para qualquer linguagem e poderia ser uma utilidade genérica na `base_verifier` ou em `lib/common.py`.

#### 3. Data over Machinery (Filosofia do Projeto)
*   Em vez de métodos como `_consecutive_newlines`, `_return_spacing`, etc., poderíamos ter uma declaração de dados (lista de tuplas/objetos de regra) e um motor (machinery) escondido na base que as processa.

#### 4. Semantic Elevation
*   As regras de "espaçamento de include" no C++ são declaradas como strings de regex brutas. Poderíamos elevar isso semanticamente para algo como `Rule.ensure_spacing_around( pattern, before=3, after=3 )`.


Notei que você mencionou que o SRP se aplica ao objeto/classe/função e não ao arquivo. No caso de `base_verifier.py`, a função `run_verifier` é o principal ponto de violação de SRP. Além disso, as classes `formatter` em C++ e Python poderiam ser puramente declarações de dados se a base fosse mais "inteligente".
