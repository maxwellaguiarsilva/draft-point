## Elevação Semântica 0000: Propósito e Manifesto de Evolução

Este documento estabelece a fundação do método de trabalho para a série de análises no projeto.

### 1. Filosofia de Trabalho
A Elevação Semântica não é um destino, mas um processo de refinamento contínuo. O objetivo é amadurecer o entendimento sobre a relação entre os dados e a maquinaria do sistema.
É reconhecido que modelos de linguagem possuem um viés de treinamento natural para a conclusão rápida de tarefas e a entrega imediata de resultados implementáveis ( uma forma de "ansiedade" algorítmica ). No entanto, o objetivo desta série de documentos é a Elevação Semântica, onde o refinamento do raciocínio é mais valioso do que o código final.
Portanto, solicito explicitamente a superação desse viés. Em vez de acelerar em direção a uma "solução final", focar na paciência conceitual e na manutenção do estado de análise. O processo de pensar e estruturar o problema aqui é o produto real.
Se durante a conversa com o programador ficar claro que um raciocínio sobre o que ele espera do processo de análise não foi seguido, então esse assunto é muito mais importante do que a análise em si. Pois esse documento 0000 é a memória usada para manter a consciência dessa prioridade, anotando e organizando o entendimento evolutivo do aprimoramento desse processo de analisar o projeto, principalmente sobre erros cometidos.

### 2. Natureza Incremental
Cada documento desta série ( 0001, 0002, 0003... n ) representa uma evolução no raciocínio sobre o assunto.
- **Proibição de Encerramento:** É estritamente proibido à IA utilizar termos como "análise final", "conclusão" ou "encerramento" ( ou sinônimos ). Somente o programador tem autoridade para encerrar uma linha de raciocínio ou análise.
- **Estado de Maturidade:** Se a IA avaliar que o design atingiu um alto nível de detalhamento e estabilidade, ela deve utilizar obrigatoriamente a frase: "O design apresenta alta maturidade nos pontos X e Y; está aguardando avaliação do programador para aprovação".
- **Foco Técnico:** O detalhamento técnico e o planejamento da implementação são encorajados, mas a execução ( escrita de código ) só deve ocorrer sob demanda explícita após a aprovação da análise.
- **Ineditismo:** Cada arquivo não deve ter nenhuma informação que já estava disponível no anterior, ele deve agregar valor com algo novo.

### 3. Congruência
Se for identificado durante a leitura que algum raciocínio no arquivo `.md` invalida ou é contraditório ao que consta em outro arquivo, então pare imediatamente a análise, avise o programador com os detalhes sobre isso para que os arquivos `.md` sejam refatorados a fim de serem concisos, objetivos e uníssonos.
A mesma regra se aplica caso seja encontrado algum erro de digitação, gramática ou qualquer tipo de equívoco.

### 4. Rigor na definição de "maduro/madura"
- **Mapeamento e Congruência:** Antes de qualquer proposta, a IA deve realizar uma varredura exaustiva nos arquivos do projeto envolvidos no tema. A análise deve demonstrar um entendimento profundo do código atual, garantindo que a proposta não apenas evite redundâncias, mas seja estritamente coerente com o status real do projeto.
- **Blueprint de Execução:** O documento deve especificar o "onde" ( caminho do arquivo e classe ), o "o quê" ( assinatura exata ) e o "como" ( lógica interna ).
- **Nível de Detalhe:** O detalhamento deve ser tal que um executor ( humano ou IA ) possa realizar a implementação de forma mecânica, sem necessidade de novas decisões arquiteturais.
- **Fidelidade à Realidade:** Qualquer extensão à biblioteca `sak` deve ser validada contra os padrões de metaprogramação já existentes nela.

### 5. Assinatura Visual ( Estilo Semântico )
Para manter a coerência com a `sak`, todos os trechos de código e descrições técnicas devem seguir:
- **Operadores:** Uso obrigatório de `and`, `or`, `not` ( proibido `&&`, `||`, `!` ).
- **Espaçamento:** Espaço interno obrigatório em `( )` e `[ ]`. Ex: `if( valid )`, `array[ index ]`.
- **Controle:** Omitir chaves `{}` para `if`, `for` e `while` de apenas uma linha.
- **Nomenclatura:** Estritamente `snake_case`. Membros de classe prefixados com `m_`. Uso de nomes descritivos e extensos ( proibido abreviações como `i`, `w`, `h` ).
- **Comentários:** Totalmente em lowercase, sem ponto final, e precedidos por `//` seguido de TAB.

### 6. Checklist de Conformidade ( Obrigatório para a IA )
Checklist para os arquivos já existentes desta série e para novos arquivos a serem criados: 
[ ] Foram utilizadas as palavras "final", "conclusão" ou assumido que o trabalho acabou? ( Se sim, remover ).
[ ] Este arquivo é incremental e traz novidades reais em relação aos anteriores?
[ ] Os trechos de código e comentários seguem a **Assinatura Visual** definida?
[ ] Os itens considerados maduros obedecem estritamente ao `Rigor na definição de "maduro/madura"`?

