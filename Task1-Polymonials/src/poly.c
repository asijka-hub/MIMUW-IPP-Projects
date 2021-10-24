/**
 * Duze zadanie czesc 1
 * autor:Andrzej Sijka
 */

/**
 * W mojej reprezentacji zakladam ze
 * 1. wielomiany w Poly sa posortowane malejaca
 * oraz
 * 2. wielomian staly mozeby jedynie typu Poly ( a bedzie miec coeff i arr = NULL)
 */

#include "poly.h"
#include <stdlib.h>

typedef long scalar;

/**
 * Zwraca wieksza z dwoch liczb
 * @param a
 * @param b
 * @return max(a,b)
 */
static int Max(int a, int b) {
    return a >= b ? a : b;
}

/**
 * Zwraca liczbe podniesiona do danej potegi przy uzyciu algorytmu szybkiego potegowania
 * @param base
 * @param power
 * @return base ^ powers;
 */
static long FastPower(long base,long power) {
    long result = 1;

    while (power > 0) {
        if (power % 2 == 0) {
            power /= 2;
            base = base * base;
        }
        else {
            power--;
            result = result * base;
            power /=2 ;
            base = base * base;
        }
    }

    return result;
}

/**
 * sprawdza czy wielomian spelnia 2 niezmiennik oraz czy nie zostal przekazany nam NULL pointer
 * @param p wielomian
 */
static void PolyCheckRepresentation(const Poly *p) {
    assert(p != NULL);
    if (!PolyIsCoeff(p))
        assert(!(PolyIsCoeff(&p->arr[0].p) && MonoGetExp(&p->arr[0]) == 0));
}
/**
 * alokuje pamiec jesli wystapil blad zwraca -1
 * @param n
 * @return bezpieczny wskaznik na zaalokowana pamiec
 */
static void* safeMalloc(size_t n)
{
    void* p = malloc(n);
    if (!p)
        exit(1);

    return p;
}
/**
 * realokuje przekazana ilosc pamiec jesli wystapil blad zwraca -1
 * @param x adres pamieci do zmienienia
 * @param n wielkosc
 * @return bezpieczny wskaznik na zrealokowana pamiec
 */
static void* safeRealloc(void* x, size_t n)
{
    void* p = realloc(x, n);
    if (!p)
        exit(1);

    return p;
}

void PolyDestroy(Poly *p) {
    PolyCheckRepresentation(p);

    if (PolyIsCoeff(p))
        return;

    for (size_t i = 0; i < p->size; ++i) {
        MonoDestroy(&p->arr[i]);
    }

    free(p->arr);
}

/**
 * Mnozy wielomian przez skalar oraz zwraca pelna gleboka kopie wyniku
 * @param p wielomian
 * @param scalar skalar przez ktory mnozymy
 * @return p * scalar
 */
Poly PolyMulByScalar(const Poly *p, scalar scalar) {
    PolyCheckRepresentation(p);

    if (scalar == 0)
        return PolyZero();

    if (PolyIsCoeff(p))
        return PolyFromCoeff(p->coeff * scalar);

    size_t newSize = 0;

    Mono *monosTmp = (Mono*) safeMalloc(sizeof (Mono) * (p->size + 1));

    for (size_t i = 0; i < p->size; ++i) {
        Poly polyTmp = PolyMulByScalar(&p->arr[i].p, scalar);
        if (!PolyIsZero(&polyTmp)) {
            monosTmp[newSize] = (Mono) {.exp = p->arr[i].exp, .p = polyTmp};
            newSize++;
        }
    }

    if (newSize == 0) {
        free(monosTmp);
        return PolyZero();
    }

    // zapewniamy ze zachowany bedzie drugi niezmiennik
    if (newSize == 1 && PolyIsCoeff(&monosTmp[0].p) && MonoGetExp(&monosTmp[0]) == 0) {
        Poly c = PolyFromCoeff(monosTmp[0].p.coeff);
        free(monosTmp);
        return c;
    }

    return (Poly) {.size = newSize, .arr = monosTmp};
}

Poly PolyClone(const Poly *p) {
    PolyCheckRepresentation(p);
    return PolyMulByScalar(p,1);
}

/**
 * Do wielomianu dodaje stala typu Coeff
 * @param p wielomian
 * @param coeff wielomian staly
 */
static void PolyAddCoeff(Poly *p, poly_coeff_t coeff) {
    PolyCheckRepresentation(p);

    assert(!PolyIsCoeff(p));

    if (coeff == 0)
        return;

    if (MonoGetExp(&p->arr[p->size - 1]) == 0) {
        if (PolyIsCoeff(&p->arr[p->size - 1].p)) {
            // w przyadku gdy ostatnia potega to to 0 i .p to 0 wystarczy ze zmniejszymy size
            // jes to rownowazne z usunieciem ostatniego elementu
            if (p->arr[p->size - 1].p.coeff + coeff == 0)
                p->size--;
            else
                p->arr[p->size - 1].p.coeff += coeff;
        }
        else
            // jesli ostatni element tablicy nie byl stala rekurencyjnie wchodzimy do .p ostatniego elementu
            PolyAddCoeff(&p->arr[p->size - 1].p, coeff);
    }
    else {
        p->size++;
        p->arr = (Mono*) safeRealloc(p->arr, sizeof (Mono) * p->size);
        p->arr[p->size - 1] = (Mono) {.exp = 0, .p = PolyFromCoeff(coeff)};
    }
}

/**
 * Szczegolny przypadek PolyAddMulByScalar gdy ktorys z wielomianow jest Coeff
 * @param p pierwszy wielomian do dodania i wymnozenia przez skalar
 * @param scalarP skalar przez ktory mnozymy pierwszy wielomian
 * @param q drugi wielomian do dodania i wymnozenia przez skalar
 * @param scalarQ skalar przez ktory mnozymy drugi wielomian
 * @return p * scalarP + q * scalarQ
 */
static Poly PolyAddMulByScalarCoeffCase(const Poly *p, scalar scalarP, const Poly *q, scalar scalarQ) {
    PolyCheckRepresentation(p);
    PolyCheckRepresentation(q);

    if (PolyIsCoeff(p) && PolyIsCoeff(q))
        return PolyFromCoeff(p->coeff * scalarP + q->coeff * scalarQ);

    Poly result;

    if (PolyIsCoeff(p)) {
        result = PolyMulByScalar(q, scalarQ);

        if (PolyIsCoeff(&result))
            return PolyFromCoeff(result.coeff + scalarP * p->coeff);

        PolyAddCoeff(&result, scalarP * p->coeff);
    }

    if (PolyIsCoeff(q)) {
        result = PolyMulByScalar(p, scalarP);

        if (PolyIsCoeff(&result))
            return PolyFromCoeff(result.coeff + scalarQ * q->coeff);

        PolyAddCoeff(&result, scalarQ * q->coeff);
    }

    return result;

}
/**
 * Mnozy wielomian typu Mono przez skalar
 * @param m wielomian typu Mono
 * @param scalar skalar
 * @return m * scalar
 */
static Mono MonoMulByScalar(Mono* m, scalar scalar) {
    return (Mono) {.exp = m->exp, .p = PolyMulByScalar(&m->p, scalar)};
}

/**
 * Dodaje do siebie dwa wielomiany wymnozone przez odpowiednie skalary
 * @param p pierwszy wielomian do dodania i wymnozenia przez skalar
 * @param scalarP skalar przez ktory mnozymy pierwszy wielomian
 * @param q drugi wielomian do dodania i wymnozenia przez skalar
 * @param scalarQ skalar przez ktory mnozymy drugi wielomian
 * @return p * scalarP + q * scalarQ
 */
static Poly PolyAddMulByScalar(const Poly *p, scalar scalarP, const Poly *q, scalar scalarQ) {
    PolyCheckRepresentation(p);
    PolyCheckRepresentation(q);

    // rozpatrzamy przypadek gdy ktory jest Coeff
    if (PolyIsCoeff(p) || PolyIsCoeff(q))
        return PolyAddMulByScalarCoeffCase(p , scalarP, q, scalarQ);

    size_t i = 0, iP = 0, iQ = 0;
    Mono *monosTmp = (Mono*) safeMalloc(sizeof (Mono) * (p->size + q->size + 1));

    // przechodzimy rownolegle dwa wielomiany wiemy juz ze oba nie sa Coeff
    while (iP <  p->size && iQ < q->size) {
        if (p->arr[iP].exp == q->arr[iQ].exp) {
            Mono tmp = (Mono) {.exp = p->arr[iP].exp,
                    .p = PolyAddMulByScalar(&p->arr[iP].p, scalarP, &q->arr[iQ].p, scalarQ)};

            if (!PolyIsZero(&tmp.p))
                monosTmp[i++] = tmp;

            iP++;
            iQ++;
        }
        else if (p->arr[iP].exp > q->arr[iQ].exp) {
            Mono tmp = MonoMulByScalar(&p->arr[iP], scalarP);

            if (!PolyIsZero(&tmp.p))
                monosTmp[i++] = tmp;

            iP++;
        }
        else {
            Mono tmp = MonoMulByScalar(&q->arr[iQ], scalarQ);

            if (!PolyIsZero(&tmp.p))
                monosTmp[i++] = tmp;

            iQ++;
        }
    }

    // rozpatrzamy przypadki gdy po rownoleglym przejsciu wielomianow zostala tylko koncowka ktoregos z nich

    if (iP == p->size) {
        while (iQ < q->size) {
            Mono tmp = MonoMulByScalar(&q->arr[iQ], scalarQ);
            if (!PolyIsZero(&tmp.p))
                monosTmp[i++] = tmp;
            iQ++;
        }
    }
    else if (iQ == q->size) {
        while (iP < p->size) {
            Mono tmp = MonoMulByScalar(&p->arr[iP], scalarP);
            if (!PolyIsZero(&tmp.p))
                monosTmp[i++] = tmp;
            iP++;
        }
    }

    if (i == 0) {
        free(monosTmp);
        return PolyZero();
    }

    // musimy zapewnic ze zachowany bedzie drugi niezmiennik
    if(i == 1 && PolyIsCoeff(&monosTmp[0].p) && MonoGetExp(&monosTmp[0]) == 0) {
        Poly result = PolyFromCoeff(monosTmp[0].p.coeff);
        free(monosTmp);
        return result;
    }

    return (Poly) {.size = i, .arr = monosTmp};
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    PolyCheckRepresentation(p);
    PolyCheckRepresentation(q);
    return PolyAddMulByScalar(p ,1 ,q ,1);
}

/**
 * komparator expow dwoch elementow typy Mono
 * @param p1 pierwszy wielomian
 * @param p2 drugi wielomian
 * @return ujemna liczba jesli pierwszy jest wiekszy, zero jest sa rowne dodatnia jesli drugi byl wiekszy
 */
static int MonosComparator(const void * p1, const void * p2) {
    if ((*(Mono*)p2).exp > (*(Mono*)p1).exp)
        return 1;
    else if ((*(Mono*)p2).exp < (*(Mono*)p1).exp)
        return -1;
    else
        return 0;
}

Poly SortMonosTabCheckSecondInvariant(size_t numberOfLeft, size_t count, Mono* monosTab) {
    // jesli skrocilismy wszystko zwracamy PolyZero
    if (numberOfLeft == 0) {
        free(monosTab);
        return PolyZero();
    }

    // sortujemy aby 'smieci' znalazly sie na koncu
    qsort(monosTab, count, sizeof (Mono), MonosComparator);

    // zapewniamy poprawnosc drugiego niezmiennika
    if (numberOfLeft == 1 && PolyIsCoeff(&monosTab[0].p) && MonoGetExp(&monosTab[0]) == 0) {
        Poly result = PolyFromCoeff(monosTab[0].p.coeff);
        free(monosTab);
        return result;
    }

    // zwracamy w  wyniku Poly z tablica ktora modyfikowalismy z nowym sizem dzieki ktoremu bedziemy wiedziec
    // ile elementow jest istotnych a ile to smiecie np elementy z wykladnikiem -1.
    return (Poly) {.size = numberOfLeft, .arr = monosTab};
}

static void HandleEaqulExp(Mono* monosTab, size_t* numberOfLeft, size_t i) {
    Mono tmp = monosTab[i + 1];
    monosTab[i + 1].p = PolyAdd(&monosTab[i].p, &tmp.p);
    monosTab[i].exp = -1;

    PolyDestroy(&monosTab[i].p);
    PolyDestroy(&tmp.p);

    (*numberOfLeft)--;
}

Mono ReturnMono(const Mono monosTab[], size_t i) {
    return monosTab[i];
}

Mono ReturnCloneMono(const Mono monosTab[], size_t i) {
    return MonoClone(&monosTab[i]);
}

Poly PolyAddMonosAndCloneMonos(size_t count, const Mono monos[], Mono (*operation)(const Mono monosTab[], size_t i)) {
    if (count == 0 || monos == NULL)
        return PolyZero();

    // rezerwujemy dodatkowe miejscie w tablicy tymczasowej by nie miec problemow w przechodzeniu petla
    Mono* monosTmp = (Mono*) safeMalloc((count + 1) * sizeof(Mono));

    size_t numberOfLeft = count;

    for (size_t i = 0; i < count; ++i) {
        monosTmp[i] = operation(monos, i);
    }

    // ten element po posortowaniu bedzie na samym koncu co ulatwi przejscie tablicy petla
    monosTmp[count] = (Mono) {.exp = -1};

    // sortujemy tablice mamy pewnosc ze wykladniki sa posortowane malejaca
    qsort(monosTmp, count + 1, sizeof (Mono), MonosComparator);

    for (size_t i = 0; i < count; ++i) {
        assert(monosTmp[i].exp >= monosTmp[i+1].exp);

        Mono curr = monosTmp[i];
        Mono next = monosTmp[i+1];

        // w ponizszych ifach w obu przypadkach gdy nie chcemy brac jakis elemntow pod uwage w wyniku
        // ustawiamy im exp -1 dzieki temu po ponownym posortowaniu znajda sie na koncu

        if (PolyIsZero(&curr.p)) {
            monosTmp[i].exp = -1;
            numberOfLeft--;
        }
        else if(MonoGetExp(&curr) == MonoGetExp(&next))
            HandleEaqulExp(monosTmp, &numberOfLeft, i);
    }

    return SortMonosTabCheckSecondInvariant(numberOfLeft, count + 1, monosTmp);
}

Poly PolyAddMonos(size_t count, const Mono monos[]) {
    return PolyAddMonosAndCloneMonos(count, monos, ReturnMono);
}

Poly PolyCloneMonos(size_t count, const Mono monos[]) {
    return PolyAddMonosAndCloneMonos(count, monos, ReturnCloneMono);
}

Poly PolyOwnMonos(size_t count, Mono *monos) {
    if (count == 0 || monos == NULL)
        return PolyZero();

    qsort(monos, count, sizeof(Mono), MonosComparator);

    size_t numberOfLeft = count;

    for (size_t i = 0; i < count; ++i) {
        if (PolyIsZero(&monos[i].p)) {
            monos[i].exp = -1;
            numberOfLeft--;
        }
        else if (i < count - 1) {
            assert(monos[i].exp >= monos[i+1].exp);

            if (MonoGetExp(&monos[i]) == MonoGetExp(&monos[i+1]))
                HandleEaqulExp(monos, &numberOfLeft, i);
        }
    }

    return SortMonosTabCheckSecondInvariant(numberOfLeft, count, monos);
}

Poly PolyMul(const Poly *p, const Poly *q) {
    PolyCheckRepresentation(p);
    PolyCheckRepresentation(q);

    if (PolyIsZero(p) || PolyIsZero(q))
        return PolyZero();
    else if (PolyIsCoeff(p) || PolyIsCoeff(q)) {
        if (PolyIsCoeff(p))
            return PolyMulByScalar(q, p->coeff);

        if (PolyIsCoeff(q))
            return PolyMulByScalar(p, q->coeff);
    }
    else {
        // rezerwujmy miejscie na wszystkie wielomiany monos jakie otrzymamy w wyniku mnozenia nastepnie je dodamy
        // uzywajac PollyAddMonos
        Mono *monosTmp = (Mono*) safeMalloc(sizeof (Mono) * (p->size * q->size));

        int monosArraySize = 0;

        for (size_t i = 0; i < p->size; ++i) {
            for (size_t j = 0; j < q->size; ++j) {
                monosTmp[monosArraySize] = (Mono) {.exp = MonoGetExp(&p->arr[i]) + MonoGetExp(&q->arr[j]),
                        .p = PolyMul(&p->arr[i].p, &q->arr[j].p)};
                monosArraySize++;
            }

        }

        Poly result = PolyAddMonos(monosArraySize, monosTmp);
        free(monosTmp);
        return result;

    }

    return PolyZero();
}

Poly PolyNeg(const Poly *p) {
    PolyCheckRepresentation(p);
    return PolyMulByScalar(p, -1);
}


Poly PolySub(const Poly *p, const Poly *q) {
    PolyCheckRepresentation(p);
    PolyCheckRepresentation(q);
    return PolyAddMulByScalar(p, 1, q ,-1);
}
/**
 * zwraca deg wielomianu typu Mono
 * @param p wielomian typu Mono
 * @return
 */
static poly_exp_t MonoDeg(const Mono *p) {
    return PolyDeg(&p->p) + p->exp;
}

poly_exp_t PolyDeg(const Poly *p) {
    PolyCheckRepresentation(p);

    if(PolyIsZero(p))
        return -1;
    else if (PolyIsCoeff(p))
        return 0;
    else {
        int max=-1;
        for (size_t i = 0; i < (*p).size; ++i) {
            max = Max(max, MonoDeg(&p->arr[i]));
        }
        return max;
    }
}
/**
 * zwraca Deg ze wzgledu na dana zmienna wielomiany typu Mono
 * @param p wielomian typu Mono
 * @param varIdx indeks zmiennej indeksowany na takiej samej zasadzie jak w PolyDegaBy
 * @return
 */
static poly_exp_t MonoDegBy(const Mono *p, size_t varIdx) {
    if (varIdx == 0)
        return p->exp;
    else
        return PolyDegBy(&p->p, varIdx - 1);
}


poly_exp_t PolyDegBy(const Poly *p, size_t varIdx) {
    PolyCheckRepresentation(p);

    if (varIdx == 0) {
        if(PolyIsZero(p))
            return -1;
        else if (PolyIsCoeff(p))
            return 0;
        else {
            int max=-1;
            for (size_t i = 0; i < (*p).size; ++i) {
                max = Max(max, p->arr[i].exp);
            }
            return max;
        }
    }
    else {
        if(PolyIsZero(p))
            return -1;
        else if (PolyIsCoeff(p))
            return 0;
        else {
            int max = -1;
            for (size_t i = 0; i < p->size; ++i) {
                max = Max(max, MonoDegBy(&p->arr[i], varIdx ));
            }
            return max;
        }
    }
}
/**
 * sprawdza rownosc dwoch wielomianow typu Mono
 * @param p pierwszy wielomian typu Mono
 * @param q drugi wielomian typu Mono
 * @return true jesli sa prawdziwe false w przeciwnym wypadku
 */
static bool MonoIsEq(const Mono *p, const Mono *q) {
    return (*p).exp == (*q).exp && PolyIsEq(&(*p).p,&(*q).p);
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    PolyCheckRepresentation(p);
    PolyCheckRepresentation(q);

    if ((*p).arr == NULL && (*q).arr == NULL && (*p).coeff == (*q).coeff)
        return true;
    else if ((*p).arr != NULL && (*q).arr != NULL && (*p).size == (*q).size){
        bool res = true;

        for (size_t i = 0; i < (*p).size; ++i) {
            res = res && MonoIsEq((*p).arr+i, (*q).arr+i);
        }

        return res;
    }
    else
        return false;
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    PolyCheckRepresentation(p);

    if (PolyIsCoeff(p))
        return PolyFromCoeff(p->coeff);
    else {
        Poly result = PolyZero();

        for (size_t i = 0; i < p->size; ++i) {
            long s = FastPower(x, p->arr[i].exp);
            if (s == 0) {
                continue;
            }
            Poly saved = result;
            result = PolyAddMulByScalar(&result, 1, &p->arr[i].p, s);
            PolyDestroy(&saved);
        }

        return result;
    }
}

Poly PolyRaisedToPower(const Poly *p, poly_exp_t exp) {
    PolyCheckRepresentation(p);

    if (exp == 0)
        return PolyFromCoeff(1);

    if (PolyIsZero(p))
        return PolyZero();

    Poly res = PolyFromCoeff(1);
    Poly mulTmp= PolyClone(p);

    while (exp > 0) {
        if (exp % 2 == 0) {
            Poly tmp = PolyMul(&mulTmp, &mulTmp);

            PolyDestroy(&mulTmp);
            mulTmp = tmp;

            exp /= 2;
        }
        else {
            if (PolyIsZero(&mulTmp) || PolyIsZero(&res)) {
                PolyDestroy(&mulTmp);
                return res;
            }
            Poly tmp = PolyMul(&res, &mulTmp);

            PolyDestroy(&res);

            res = tmp;
            exp--;
        }
    }

    PolyDestroy(&mulTmp);
    return res;
}

Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {
    PolyCheckRepresentation(p);

    if (PolyIsCoeff(p))
        return PolyFromCoeff(p->coeff);

    if (k == 0) {
        if (MonoGetExp(&p->arr[p->size - 1]) == 0)
            return PolyCompose(&p->arr[p->size - 1].p, 0, q);
        else
            return PolyZero();
    }

    Poly res = PolyZero();
    for (size_t i = 0; i < p->size; ++i) {
        Poly poly1 = PolyRaisedToPower(&q[k - 1], MonoGetExp(&p->arr[i]));
        Poly poly2 = PolyCompose(&p->arr[i].p, k - 1, q);
        Poly poly3 = PolyMul(&poly1, &poly2);
        Poly tmp = PolyAdd(&res, &poly3);

        PolyDestroy(&poly1);
        PolyDestroy(&poly2);
        PolyDestroy(&poly3);
        PolyDestroy(&res);

        res = tmp;
    }

    return res;
}