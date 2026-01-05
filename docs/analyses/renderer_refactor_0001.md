## Refatoração do Renderizador 0001: Fundamentos e Fronteiras de Domínio

Esta análise consolida o diagnóstico inicial e estabelece as bases para a separação entre geometria pura e mecanismos de exibição.

### 1. Diagnóstico: O Abismo Semântico
O renderizador atual opera em um nível predominantemente mecânico, desmembrando entidades matemáticas ( como `sak::point` ) em coordenadas escalares para cálculos manuais. A refatoração visa eliminar essa "maquinaria" algorítmica em favor de uma linguagem vetorial e declarativa.

### 2. Responsabilidade e Linearização
A conversão de coordenadas 2D para um índice linear de buffer é uma preocupação de domínio do renderizador, mas deve ser expressa através da álgebra da `sak`.
- **Agnosticismo Espacial:** `sak::point` permanece agnóstico ao layout de memória.
- **Projeção Escalar:** A indexação do buffer é tratada como uma soma de produtos ( strides ), elevando a semântica:
```cpp
point strides = { 1, width };
int index = sak::math::sum( local_position * strides );
```

### 3. O Conceito de Superfície ( Surface )
Introduzimos a abstração de "Superfície" como a tradutora final entre o espaço geométrico e a memória.
- **Papel:** Encapsular a aritmética de ponteiros e o layout do buffer.
- **Interface:** Oferecer acesso via `operator [ ]( point )`, permitindo que o algoritmo de desenho ignore a estrutura unidimensional subjacente.

### 4. Expansões Necessárias na Infraestrutura (`sak`)
Para suportar este design, a `sak` deve prover:
1.  **Transformações Unárias ( `map` ):** Para aplicar funções como `abs` ou `sign` a componentes de pontos.
2.  **Reduções Lógicas:** Métodos como `is_between( )` e `clamp( )` para validação de limites espaciais.
3.  **Matemática Vetorial:** Inclusão de `sak::math::sum( )` e `sign( )` compatíveis com pontos.
