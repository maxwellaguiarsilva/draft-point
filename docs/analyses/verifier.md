---

## MODEL ✨

O `base_verifier.py` hoje sofre do que podemos chamar de "Síndrome do Caso Especial". A arquitetura tenta ser orientada a dados (a classe `rule`), mas como essa abstração não é potente o suficiente para expressar exclusões ou agrupamentos, o "maquinário" (`run_rules`, `any_actual_change`) acaba assumindo o controle com diversos "puxadinhos".

### Diagnóstico da Situação

1.  **Fragmentação do Protocolo de Aplicação**: Existem hoje quatro caminhos distintos para aplicar regras:
    *   **Regex Direto (`_apply`)**: Aplicação atômica com mensagens individuais.
    *   **Agrupado com Exclusão (`_apply_with_exclusion`)**: Um caminho especial para colchetes que introduz conceitos como "split de header/body", padrões de exclusão e mensagens de sumário (`fix_message`).
    *   **Lógica Hardcoded (`_validate_license`, `_trailing_newlines`)**: Cidadãos de "primeira classe" que rodam fora do motor de regras, tornando impossível tratá-los de forma genérica.
    *   **Orquestração por Tipo (`run_rules`)**: O motor de regras tem conhecimento íntimo de nomes de chaves (`bracket_ignore`) e tipos (`isinstance(item, list)`), o que é o oposto de "Data over Machinery".

2.  **Redundância de Rastreamento de Estado**: O `any_actual_change` e o `first_line` dentro de `_apply_with_exclusion` são reconstruções manuais de informações que já existem (a diferença entre strings e a lista de mensagens). Isso espelha o `f.content != new_content` do `run_verifier`, criando camadas redundantes de detecção de mudança.

3.  **Assimetria de Escopo**: Apenas as regras que passam por `_apply_with_exclusion` respeitam a separação entre licença e corpo do arquivo. Isso significa que uma regra de regex comum pode "quebrar" o cabeçalho de licença, enquanto as de colchete são protegidas, uma inconsistência perigosa.

4.  **Conflito de Mensageria**: A decisão de mostrar uma mensagem detalhada ou um sumário está acoplada à forma como a regra é aplicada, e não a uma política de exibição agnóstica ao `flg_auto_fix`.

### Pequeno Passo Certo para Começar

O primeiro passo ideal é **unificar a aplicação de transformações através da comparação de dados, eliminando sinalizadores manuais**.

**Ação proposta:** Refatorar o `_apply` e o `_apply_with_exclusion` para que ambos utilizem o mesmo padrão: comparar o "conteúdo antes" com o "conteúdo depois" para decidir se devem registrar uma mudança, eliminando o `any_actual_change`. 

Isso permitiria que o `run_rules` deixasse de ser um orquestrador complexo e passasse a ser um simples iterador sobre uma lista de transformações, onde cada transformação (seja ela um regex simples, um grupo com exclusão ou a própria licença) segue o mesmo contrato: `(conteúdo_antigo) -> (conteúdo_novo, mensagens)`.

Deseja que eu explore como seria a assinatura dessa abstração unificada para começarmos a reduzir essa assimetria?

---

## USER 🧑‍💻

sim faça essa exploração
eu tenho a seguinte visão de regra de negócio:
- a restauração do cabeçalho é um passo zero, porque ele garnte a separação de header; body
- com isso se torna possível que todas as regras sejam exclusivamente no body, uma vez que o header está resolvido, ele não faz mais parte do que está acontecendo
- é esperado que algumas regras tenham ignore_pattern, para não afetar string literais e comentários, isso é pequeno e essa assimetria é premeditada
- a parte das mensagens é o que mais me incomoda de não estar em isonomia
  - não é um problema o fato que o número da linha fazia referencia ao arquivo/conteúdo antigo antes do replace
  - esse script não tem que ficar preocupado com o número da linha certa no novo conteúdo ajustado, o objetivo do número da linha é dizer onde "estava" o erro, no arquivo antigo

---

## MODEL ✨

The current architecture of `base_verifier.py` suffers from "Special Case Syndrome," where the machinery has grown to handle specific rules rather than providing a generic engine.

### Diagnosis

1.  **Step 0 is not isolated**: `_validate_license` is currently just another method. Because it doesn't formally establish the boundary between the header and the body, subsequent rules can accidentally corrupt the canonical license.
2.  **Redundant Machinery**: Concepts like `any_actual_change` and `first_line` are manual state-tracking "hacks" (puxadinhos). In a "Data over Machinery" approach, change detection should be a byproduct of string comparison (`new_content != old_content`), not a flag set in a callback.
3.  **Functional Asymmetry**:
    *   **Scope**: Simple rules (`_apply`) run on the whole file; exclusive rules (`_apply_with_exclusion`) run only on the body. This is inconsistent and dangerous for the license header.
    *   **Reporting**: Summary messages (like "fixed bracket spacing") are hardcoded into the exclusion logic instead of being a generic property of any rule group.
4.  **Fragile Line Counting**: The logic for calculating line numbers is reimplemented in multiple places, often making assumptions about the content structure that could be centralized.

### The "Small Right Step"

The most effective first step is to **enforce a strict Header/Body isolation at the pipeline level**.

Instead of having rules decide if they should split the file, the `run()` method should become a rigid orchestrator:

1.  **Normalize**: Run `_validate_license` to ensure the header is correct and the `\n\n\n` separator exists.
2.  **Isolate**: Split the content exactly once into a `header` (immutable) and a `body` (mutable).
3.  **Transform**: Apply all rules (simple or exclusive) only to the `body`. Each application simply returns a new `body` string.
4.  **Reassemble**: Join `header + body` only at the very end.

### Benefits of this Step

*   **Safety**: No rule, no matter how aggressive its regex, can ever touch the license header again.
*   **Simplicity**: The "Machinery" of `_apply_with_exclusion` (the splitting and joining) is deleted.
*   **Isonomy**: We can merge `_apply` and `_apply_with_exclusion` into a single `_apply_rules` method that optionally takes an `ignore_pattern`.
*   **Clean Reporting**: We can detect changes by comparing the `body` before and after a group of rules, allowing us to add summary messages without `nonlocal` flags.

Would you like me to draft the structural changes for this pipeline-based `run()` method?
