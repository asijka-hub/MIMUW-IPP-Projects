/** @file
  Interfejs klasy wielomianów rzadkich wielu zmiennych

  @authors Jakub Pawlewicz <pan@mimuw.edu.pl>, Marcin Peczarski <marpe@mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#ifndef __POLY_H__
#define __POLY_H__

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

/** To jest typ reprezentujący współczynniki. */
typedef long poly_coeff_t;

/** To jest typ reprezentujący wykładniki. */
typedef int poly_exp_t;

struct Mono;

/**
 * To jest struktura przechowująca wielomian.
 * Wielomian jest albo liczbą całkowitą, czyli wielomianem stałym
 * (wtedy `arr == NULL`), albo niepustą listą jednomianów (wtedy `arr != NULL`).
 */
typedef struct Poly {
    /**
    * To jest unia przechowująca współczynnik wielomianu lub
    * liczbę jednomianów w wielomianie.
    * Jeżeli `arr == NULL`, wtedy jest to współczynnik będący liczbą całkowitą.
    * W przeciwnym przypadku jest to niepusta lista jednomianów.
    */
    union {
        poly_coeff_t coeff; ///< współczynnik
        size_t       size; ///< rozmiar wielomianu, liczba jednomianów
    };
    /** To jest tablica przechowująca listę jednomianów. */
    struct Mono *arr;
} Poly;

/**
 * To jest struktura przechowująca jednomian.
 * Jednomian ma postać 'px_i^n'.
 * Współczynnik 'p' może też być
 * wielomianem nad kolejną zmienną 'x_{i+1}'.
 */
typedef struct Mono {
    Poly p; ///< współczynnik
    poly_exp_t exp; ///< wykładnik
} Mono;

/**
 * Daje wartość wykładnika jendomianu.
 * @param[in] m : jednomian
 * @return wartość wykładnika jednomianu
 */
static inline poly_exp_t MonoGetExp(const Mono *m) {
    return m->exp;
}

/**
 * Tworzy wielomian, który jest współczynnikiem (wielomian stały).
 * @param[in] c : wartość współczynnika
 * @return wielomian
 */
static inline Poly PolyFromCoeff(poly_coeff_t c) {
    return (Poly) {.coeff = c, .arr = NULL};
}

/**
 * Tworzy wielomian tożsamościowo równy zeru.
 * @return wielomian
 */
static inline Poly PolyZero(void) {
    return PolyFromCoeff(0);
}

static inline bool PolyIsZero(const Poly *p);

/**
 * Tworzy jednomian 'px_i^n'.
 * Przejmuje na własność zawartość struktury wskazywanej przez @p p.
 * @param[in] p : wielomian - współczynnik jednomianu
 * @param[in] n : wykładnik
 * @return jednomian 'px_i^n'
 */
static inline Mono MonoFromPoly(const Poly *p, poly_exp_t n) {
    assert(n == 0 || !PolyIsZero(p));
    return (Mono) {.p = *p, .exp = n};
}

/**
 * Sprawdza, czy wielomian jest współczynnikiem (czy jest to wielomian stały).
 * @param[in] p : wielomian
 * @return Czy wielomian jest współczynnikiem?
 */
static inline bool PolyIsCoeff(const Poly *p) {
    return p->arr == NULL;
}

/**
 * Sprawdza, czy wielomian jest tożsamościowo równy zeru.
 * @param[in] p : wielomian
 * @return Czy wielomian jest równy zeru?
 */
static inline bool PolyIsZero(const Poly *p) {
    return PolyIsCoeff(p) && p->coeff == 0;
}

/**
 * Usuwa wielomian z pamięci.
 * @param[in] p : wielomian
 */
void PolyDestroy(Poly *p);

/**
 * Usuwa jednomian z pamięci.
 * @param[in] m : jednomian
 */
static inline void MonoDestroy(Mono *m) {
    PolyDestroy(&m->p);
}

/**
 * Robi pełną, głęboką kopię wielomianu.
 * @param[in] p : wielomian
 * @return skopiowany wielomian
 */
Poly PolyClone(const Poly *p);

/**
 * Robi pełną, głęboką kopię jednomianu.
 * @param[in] m : jednomian
 * @return skopiowany jednomian
 */
static inline Mono MonoClone(const Mono *m) {
    return (Mono) {.p = PolyClone(&m->p), .exp = m->exp};
}

/**
 * Dodaje dwa wielomiany.
 * @param[in] p : wielomian 'p'
 * @param[in] q : wielomian 'q'
 * @return 'p + q'
 */
Poly PolyAdd(const Poly *p, const Poly *q);

/**
 * Sumuje listę jednomianów i tworzy z nich wielomian.
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyAddMonos(size_t count, const Mono monos[]);



/**
 * Sumuje listę jednomianów i tworzy z nich wielomian. Przejmuje na własność
 * pamięć wskazywaną przez @p monos i jej zawartość. Może dowolnie modyfikować
 * zawartość tej pamięci. Zakładamy, że pamięć wskazywana przez @p monos
 * została zaalokowana na stercie. Jeśli @p count lub @p monos jest równe zeru
 * (NULL), tworzy wielomian tożsamościowo równy zeru.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyOwnMonos(size_t count, Mono *monos);
/**
 * Sumuje listę jednomianów i tworzy z nich wielomian. Nie modyfikuje zawartości
 * tablicy @p monos. Jeśli jest to wymagane, to wykonuje pełne kopie jednomianów
 * z tablicy @p monos. Jeśli @p count lub @p monos jest równe zeru (NULL),
 * tworzy wielomian tożsamościowo równy zeru.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyCloneMonos(size_t count, const Mono monos[]);

/**
 * Mnoży dwa wielomiany.
 * @param[in] p : wielomian 'p'
 * @param[in] q : wielomian 'q'
 * @return 'p * q'
 */
Poly PolyMul(const Poly *p, const Poly *q);

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian 'p'
 * @return '-p'
 */
Poly PolyNeg(const Poly *p);

/**
 * Odejmuje wielomian od wielomianu.
 * @param[in] p : wielomian 'p'
 * @param[in] q : wielomian 'q'
 * @return 'p - q'
 */
Poly PolySub(const Poly *p, const Poly *q);

/**
 * Zwraca stopień wielomianu ze względu na zadaną zmienną (-1 dla wielomianu
 * tożsamościowo równego zeru). Zmienne indeksowane są od 0.
 * Zmienna o indeksie 0 oznacza zmienną główną tego wielomianu.
 * Większe indeksy oznaczają zmienne wielomianów znajdujących się
 * we współczynnikach.
 * @param[in] p : wielomian
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx
 */
poly_exp_t PolyDegBy(const Poly *p, size_t var_idx);

/**
 * Zwraca stopień wielomianu (-1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDeg(const Poly *p);

/**
 * Sprawdza równość dwóch wielomianów.
 * @param[in] p : wielomian 'p'
 * @param[in] q : wielomian 'q'
 * @return 'p = q'
 */
bool PolyIsEq(const Poly *p, const Poly *q);

/**
 * Wylicza wartość wielomianu w punkcie @p x.
 * Wstawia pod pierwszą zmienną wielomianu wartość @p x.
 * W wyniku może powstać wielomian, jeśli współczynniki są wielomianami.
 * Wtedy zmniejszane są o jeden indeksy zmiennych w takim wielomianie.
 * Formalnie dla wielomianu 'p(x_0, x_1, x_2, \ldots)' wynikiem jest
 * wielomian 'p(x, x_0, x_1, \ldots)'.
 * @param[in] p : wielomian 'p'
 * @param[in] x : wartość argumentu 'x'
 * @return 'p(x, x_0, x_1, \ldots)'
 */
Poly PolyAt(const Poly *p, poly_coeff_t x);

Poly PolyCompose(const Poly *p, size_t k, const Poly q[]);

#endif /* __POLY_H__ */
