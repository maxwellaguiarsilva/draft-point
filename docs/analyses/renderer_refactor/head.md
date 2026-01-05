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
- Identificar que tem muitos arquivos e com informações redundantes. Que eles poderiam ser sumarizados em poucos arquivos em um texto menor com maior qualidade, objetividade e concisão.

### 3. Ordem de Prioridade e Consulta Obrigatória
Sempre que uma refatoração ou nova análise for iniciada, os arquivos devem ser consultados e/ou atualizados rigorosamente nesta ordem de importância:
1.  **`head.md`**: O guia mestre e guardião da filosofia da análise atual.
2.  **`error_log.md`**: Log comportamental para evitar reincidência em falhas de conduta.
3.  **`history.md`**: Trajetória de decisões e amadurecimento ( sinalizações do programador ).
4.  **`fail_log.md`**: Impedimentos técnicos e omissões mapeadas.
5. **analysis.md**: O amadurecimento conceitual e diagnóstico de domínio.
6. **design.md**: O blueprint técnico e a arquitetura das abstrações.
7. **execution.md**: O guia mecânico para implementação cega.

### 4. Protocolo de Execução ( Qual papel cada agente de IA deve executar )
- **Analista**: Raciocina, projeta e documenta. Proibido alterar código. Produto final: execution.md de alta precisão.
- **Executor**: Segue instruções cegas de alteração de arquivo. Se o executor falhar, a análise foi imatura.
- **Revisor**: Avalia se os markdowns e os arquivos de código ( cpp e hpp ) estão em compliance com GEMINI.md, inclusive com o apoio das ferramentas MCP. Se algo não estar como deveria, registra em history.md.

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
