# Semantic Elevation: The Core Philosophy

This project is guided by the principle of **Semantic Elevation**. The goal is to lift code from being a set of imperative instructions for the CPU ("how") to a declarative description of domain intent ("what").

## 1. The Essence of Semantic Elevation
Code should exist at the highest possible level of abstraction relative to its purpose. We do not write "logic"; we describe "relationships" and "states".

- **Machinery vs. Data:** Pointers, iterators, and manual loops are *machinery*. References, objects, and ranges are *data*. Semantic Elevation means hiding the machinery to let the data speak.
- **Intent-Driven Design:** Every line of code should answer: "What am I trying to express?" rather than "What is the CPU doing?".

## 2. Visual Serenity (Noise Reduction)
Semantic clarity requires the elimination of visual noise. The syntax of C++ should not distract from the meaning.

- **Flattened Namespaces:** Use `using` blocks to bring terms into the local scope. In the code body, `transform` is a concept; `::std::ranges::transform` is an address. Addresses are noise.
- **Reference over Indirection:** Prefer `const&` over `->`. A pointer implies an address in memory (machinery); a reference implies the object itself (semantic entity).
- **Semantic Spacing:** Use single spaces inside `( )` and `[ ]` to give terms "breathing room," treating them as logical delimiters for formulas.

## 3. Composition as Grammar
We build complex logic by composing atomic, named entities (Niebloids/CPOs).

- **No Ad-hoc Logic:** If a logic block (like squaring a number or checking a condition) can be named, it must be an object in the `sak` library.
- **Lambdas as a Last Resort:** If you are writing a lambda, you are likely failing to elevate the logic. Ask: "Should this be a reusable concept in `sak`?"
- **The "Sak" Bridge:** The `sak` (Swiss Army Knife) library is the project's dictionary. It translates low-level C++ mechanics into high-level semantic tools.

## 4. Examples of Elevation (Directional Vector)

These are not strict rules to follow, but examples of the **elevated path**:

### A. Geometrical Relationship
Instead of checking `x1 <= x2` and `y1 <= y2` (technical), we declare a relationship:
```cpp
// Elevated: What is the relationship?
return all_of( zip( *this, other ), tupled( less_equal ) );
```

### B. Functional Calculation
Instead of a `for` loop with a temporary `sum` variable (mechanical), we describe the formula:
```cpp
// Elevated: What is the formula?
return square_root( sum( transform( m_data, square ) ) );
```

### C. Domain Movement
Instead of manually updating `x += 1` based on an enum (imperative), we apply the direction's inherent property:
```cpp
// Elevated: How does the domain behave?
position += use_direction( m_direction ).value;
```

### D. Pattern Retrieval
Instead of an `if( find != end )` check (technical machinery), we express the intent of a fallback:
```cpp
// Elevated: What is the desired outcome?
return value_or( m_map, key, default_value );
```

## 5. The Decision Matrix for the Agent
When faced with a task, do not look for a "case" in the manual. Instead, apply this vector:
1. **Identify the Intent:** What is the domain-level goal?
2. **Elevate the Syntax:** Can I remove `::` or `->` to make it cleaner?
3. **Compose the Solution:** Can I use `sak` tools or higher-order functions to describe the result?
4. **Name the Logic:** If I need a new tool, should I create it in `sak` first?
