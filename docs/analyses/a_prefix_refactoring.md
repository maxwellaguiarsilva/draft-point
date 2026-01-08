# Refatoração: Remoção do Prefixo `a_`

Este documento orienta a remoção do prefixo `a_` (utilizado para identificar argumentos) em todo o projeto, visando aumentar a serenidade visual e aderir a um estilo de nomenclatura mais natural, conforme discutido.

## Contexto e Regras

1.  **Macros:** **NÃO** alterar argumentos de macros (ex: em `include/sak/geometry/point.hpp`). Prefixos em macros são vitais para evitar substituições indesejadas.
2.  **Colisão de Nomes:** Em casos onde o nome semântico ideal colide com o nome do tipo (ex: `draw( const line& line )`), deve-se buscar um sinônimo ou um nome que descreva o papel do objeto.
3.  **Consistência:** Membros de classe (`m_`) e parâmetros de template (`t_`) permanecem inalterados.

## Mapeamento de Substituições Sugeridas

Abaixo estão os casos identificados e as recomendações de nomenclatura para evitar conflitos com tipos existentes.

### 1. Casos de Remoção Direta (Sem Colisão)

| Nome Atual | Novo Nome | Contexto Comum |
| :--- | :--- | :--- |
| `a_size` | `size` | Dimensões, tamanhos de tela. |
| `a_position` | `position` | Localização espacial. |
| `a_args` | `args` | Variadic templates. |
| `a_error` | `error_code` | Tratamento de exceções. |
| `a_value` | `value` | Valores genéricos em lambdas/templates. |
| `a_argument_count` | `argument_count` | Função `main`. |
| `a_argument_values` | `argument_values` | Função `main`. |

### 2. Casos com Colisão de Tipo (Necessário Sinônimo)

| Tipo | Nome Atual | Sugestão | Motivo |
| :--- | :--- | :--- | :--- |
| `direction` | `a_direction` | `heading` | Evita colisão com o tipo `direction`. |
| `line` | `a_line` | `segment` | Evita colisão com o tipo `line`. |
| `rectangle` | `a_rectangle` | `area` ou `box` | Evita colisão com o tipo `rectangle`. |
| `point` | `a_point` | `pixel` ou `target` | Evita colisão com o tipo `point`. |
| `range` | `a_range` | `subject` | Evita confusão com namespaces de ranges. |

## Instruções para o Executor

1.  Localize as ocorrências de `a_` usando `grep -rE "\ba_[a-z0-9_]+"`.
2.  Ignore qualquer linha que comece com `#define` ou que contenha argumentos de macro.
3.  Aplique a renomeação tanto na declaração (header) quanto na definição (source) e em todos os usos internos à função.
4.  Após a alteração, execute as ferramentas de verificação:
    *   `compile` (para garantir que não houve colisão de nomes que o compilador não consiga resolver).
    *   `analyze` (para manter a formatação).
    *   `verify_formatting` (para validar as regras de espaçamento).

## Exemplos de Transformação

**Antes:**
```cpp
auto renderer::draw( const line& a_line ) noexcept -> void
{
    const point difference = ( a_line.end - a_line.start );
    // ...
}
```

**Depois:**
```cpp
auto renderer::draw( const line& segment ) noexcept -> void
{
    const point difference = ( segment.end - segment.start );
    // ...
}
```
