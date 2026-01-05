## Análise Mestra: Refatoração do Renderizador ( head.md )

Este documento estabelece a fundação do método de trabalho e a governança da série de análise.

### 1. Filosofia de Trabalho
- **Elevação Semântica**: O objetivo é amadurecer o entendimento sobre a relação entre os dados e a maquinaria do sistema. Refinamento do raciocínio é a prioridade sobre o código final.
- **Hierarquia sobre Prefixos**: O contexto de uma análise é definido por sua localização na estrutura de diretórios, não por prefixos ou sufixos nos nomes dos arquivos. Nomes devem ser curtos, diretos e significativos dentro de seu escopo.
- **Memória de Consciência**: Este documento `head.md` é a cabeça da análise, servindo para manter a consciência das prioridades e organizar o entendimento evolutivo.

### 2. Congruência e Qualidade
Pare imediatamente tudo e qualquer coisa que esteja fazendo quando:
- Identificar erros de digitação, gramática ou ortografia ( corrija-os imediatamente ).
- Identificar raciocínios contraditórios entre os arquivos da série.
- Identificar violações das diretrizes deste `head.md`.
- Identificar que tem muitos arquivos e com informações redundantes, que eles poderiam ser sumarizados em poucos arquivos em um texto menor com maior qualidade, objetividade e concisão.

### 3. Ordem de Prioridade e Consulta Obrigatória
Sempre que uma refatoração ou nova análise for iniciada, os arquivos devem ser consultados e/ou atualizados rigorosamente nesta ordem de importância:
1.  **`head.md`**: O guia mestre e guardião da filosofia da análise atual.
2.  **`error_log.md`**: Log comportamental para evitar reincidência em falhas de conduta.
3.  **`history.md`**: Trajetória de decisões e amadurecimento ( sinalizações do programador ).
4.  **`fail_log.md`**: Impedimentos técnicos e omissões mapeadas.
5. **Documentos Numerados ( 0001..000N )**: A evolução técnica incremental e sumarizada.
6. **execution.md**: O guia mecânico para implementação cega.

### 4. Protocolo de Execução e Autoridade ( Quem faz o quê )
A separação de papéis é absoluta e nenhum agente deve invadir a permissão do outro.

- **Orchestrator ( Programador )**: O único com autoridade sobre a estrutura de diretórios, nomes de arquivos e orquestração geral. Define a estratégia e o "caminho" da análise.
- **Analista**: Raciocina, projeta e documenta. Sua autoridade é estritamente técnica e limitada aos arquivos numerados ( `0001..000N` ). É **proibido** criar novos arquivos fora da numeração ou alterar o código fonte. Sua função é elevar a semântica e produzir o `execution.md`.
- **Executor**: Segue instruções cegas de alteração de arquivo. Não possui autoridade de decisão; apenas executa o que está no `execution.md`.
- **Revisor**: O guardião da conformidade técnica e estilística ( `GEMINI.md` ). Sua autoridade limita-se a validar se os arquivos ( `.md`, `.cpp`, `.hpp` ) seguem as regras. Se identificar falhas, registra no `history.md`, mas **não** tem autoridade para alterar a estrutura de arquivos definida pelo Orchestrator.

**Regra de Sumarização**: Quando identificada a necessidade de sumarizar informações, esta ação deve ser executada obrigatoriamente **re-escrevendo** os arquivos numerados existentes ( começando pelo `0001.md` ). É expressamente proibido criar arquivos nomeados para sumarização sem ordem direta do Orchestrator.

### 5. Natureza Incremental e Profundidade
- Cada arquivo agrega valor sem repetir informações anteriores.
- Proibido termos de "conclusão" ou "encerramento" sem autoridade do programador.
- O detalhamento deve permitir execução mecânica ( onde, o que, como ).

### 6. Refatoração de Contexto Total
Sempre que o programador sinalizar um ponto ou trouxer novo entendimento:
- **Refatoração Obrigatória**: Todos os arquivos da série ( seguindo a ordem de prioridade ) devem ser atualizados para eliminar obsolescências.
- **Registro no Histórico**: Sinalizações do programador que gerem refatoração devem ser obrigatoriamente registradas em `history.md`.

### 7. Documentação Auxiliar ( Escopo Local )
- **`history.md`**: Evolução das decisões.
- **`error_log.md`**: Falhas de conduta/protocolo.
- **`fail_log.md`**: Falhas técnicas de implementação.
