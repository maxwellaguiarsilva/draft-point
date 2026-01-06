# Refatoração do Renderer: Oportunidades de Elevação Semântica e Modernidade

Este documento detalha as oportunidades de melhoria identificadas para o componente `tui::renderer`, focando em alinhar o código com a filosofia "Data over Machinery" e as capacidades do C++23.

## 1. Elevação Semântica (Filosofia "Data over Machinery")
O uso de loops `for` manuais com índices em `refresh` e `draw` expõe detalhes de implementação da CPU em vez da intenção do programador.
- **Proposta:** Substituir todos os loops `for( int i = 0; ... )` por `for( auto const i : iota( ... ) )`.
- **Objetivo:** Transformar a "maquinaria" de iteração em uma declaração de intervalo de dados, reduzindo o ruído visual e o risco de erros de índice.

## 2. Modernidade (C++23) e Visual Serenity
A lógica de renderização em `refresh` agrupa linhas manualmente (`2 * row` e `2 * row + 1`).
- **Proposta:** Utilizar `views::chunk( 2 )` para agrupar as linhas do buffer em pares de forma declarativa.
- **Objetivo:** Eliminar o cálculo manual de índices de linha, permitindo que a estrutura do loop reflita diretamente a natureza da renderização (onde cada célula do terminal representa dois pixels verticais).

## 4. Consistência em `draw( rectangle )`
Atualmente, existe uma discrepância de estilo entre o preenchimento de retângulos (que usa `cartesian_product` e `iota`) e o desenho de bordas (que usa loops manuais).
- **Proposta:** Unificar o desenho de bordas utilizando `iota` para as coordenadas de coluna e linha.
- **Objetivo:** Manter a consistência estética e técnica em toda a classe, reforçando o uso de ferramentas de alto nível da biblioteca `sak` e da STL.
