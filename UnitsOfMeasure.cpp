//
// Created by sigsegv on 1/16/24.
//

#include "UnitsOfMeasure.h"

class StrengthUnits : public UnitsOfMeasure {
public:
    StrengthUnits() {
        // Måleenhet for legemidlers styrke (OID=9090)
        units.insert_or_assign("Pst", "prosent");
        units.insert_or_assign("%", 	"prosent");
        units.insert_or_assign("PstVV", "prosent angitt som volum per volum");
        units.insert_or_assign("% v/v", 	"prosent angitt som volum per volum");
        units.insert_or_assign("PstWV", "prosent gitt som vekt per volum");
        units.insert_or_assign("% w/v", 	"prosent gitt som vekt per volum");
        units.insert_or_assign("PstWW", "prosent gitt som vekt per vekt");
        units.insert_or_assign("% w/w", 	"prosent gitt som vekt per vekt");
        units.insert_or_assign("10E10 bakterier", 	"10 opphøyd i 10'ende bakterier");
        units.insert_or_assign("10E9 bakterier", 	"10 opphøyd i 9'ende bakterier");
        units.insert_or_assign("AGNU", 	"Allergan unit");
        units.insert_or_assign("AgU", 	"antigen enheter");
        units.insert_or_assign("antigen enheter", 	"antigen enheter");
        units.insert_or_assign("anti-heparin IE", 	"anti-heparin internasjonale enheter");
        units.insert_or_assign("AU", 	"allergen unit(s)");
        units.insert_or_assign("AU A.Artemisiifolia", 	"Allergen unit for Ambrosia Artemisiifolia");
        units.insert_or_assign("CCID50", 	"Cell Culture Infective Dose 50%");
        units.insert_or_assign("cfu", 	"colony forming units");
        units.insert_or_assign("D-antigen enheter", 	"Enheter av smittsomme poliovirus");
        units.insert_or_assign("E", 	"enheter");
        units.insert_or_assign("EID50", 	"50% Embryo Infective Dose. The titre required to infect 50% of the embryos inoculated with the virus.");
        units.insert_or_assign("ELISA E", 	"ELISA-enheter");
        units.insert_or_assign("ELISA RU", 	"relative ELISA-enheter");
        units.insert_or_assign("g", 	"gram");
        units.insert_or_assign("g Ca", 	"gram kalsium");
        units.insert_or_assign("Genomkopier", 	"Genomkopier (GC)");
        units.insert_or_assign("GBq", 	"gigabequerel");
        units.insert_or_assign("HEP", 	"Histaminekvivalenter basert på prikktesting");
        units.insert_or_assign("HU", 	"haemagglutinating units");
        units.insert_or_assign("IC", 	"konsentrasjonsindeks");
        units.insert_or_assign("IE", 	"internasjonale enheter");
        units.insert_or_assign("IE anti-Xa", 	"internasjonale enheter antistoff");
        units.insert_or_assign("IR", 	"reaktivitetsindeks");
        units.insert_or_assign("IU", 	"international units");
        units.insert_or_assign("kBq", 	"kilobequerel");
        units.insert_or_assign("kg", 	"kilogram");
        units.insert_or_assign("KIU", 	"kallidinogenase inactivator unit");
        units.insert_or_assign("kJ", 	"kilojoule");
        units.insert_or_assign("LD50 enheter", 	"den statistisk bestemte dødelige dosen hos 50 % av dyrene");
        units.insert_or_assign("log10 ELISA enhet", 	"log10 enzyme-linked immunosorbent assay unit");
        units.insert_or_assign("MBq", 	"megabequerel");
        units.insert_or_assign("mg", 	"milligram");
        units.insert_or_assign("mg alfa-TE", 	"milligram alfatokoferol-ekvivalent");
        units.insert_or_assign("mg Ca", 	"milligram kalsium");
        units.insert_or_assign("mg F", 	"milligram fluor");
        units.insert_or_assign("mg Fe", 	"milligram jern");
        units.insert_or_assign("mg FNE", 	"milligram fenytoinnatriumekvivalenter");
        units.insert_or_assign("mg I", 	"milligram iod");
        units.insert_or_assign("mg K", 	"milligram kalium");
        units.insert_or_assign("mg Mg", 	"milligram magnesium (ion)");
        units.insert_or_assign("mg NE", 	"milligram niacinekvivalent");
        units.insert_or_assign("mg Zn", 	"milligram sink");
        units.insert_or_assign("mikrog", 	"mikrogram");
        units.insert_or_assign("mikrog HA", 	"mikrogram haemagglutinin");
        units.insert_or_assign("mikrog RE", 	"mikrogram retinolekvivalent");
        units.insert_or_assign("mikroliter", 	"mikroliter");
        units.insert_or_assign("mikromol", 	"mikromol");
        units.insert_or_assign("mill celler", 	"millioner celler");
        units.insert_or_assign("mill E", 	"millioner enheter");
        units.insert_or_assign("mill IE", 	"millioner internasjonale enheter");
        units.insert_or_assign("ml", 	"milliliter");
        units.insert_or_assign("mmol", 	"millimol");
        units.insert_or_assign("mmol Li", 	"millimol Litium");
        units.insert_or_assign("mol", 	"mol");
        units.insert_or_assign("nanogram", 	"nanogram");
        units.insert_or_assign("Normal", 	"normal");
        units.insert_or_assign("PFU", 	"plaque forming units");
        units.insert_or_assign("PhEur. enheter", 	"enheter etter standard i den europeiske farmakope");
        units.insert_or_assign("ppm (mol pr. mol)", 	"parts per million beregnet som mol/mol");
        units.insert_or_assign("RPS", 	"Relative Percentage Survival");
        units.insert_or_assign("SA.U", 	"tilstrekkelig mengde til å oppnå et titer av agglutinerende antistoff på 1 log10 i marsvin");
        units.insert_or_assign("SN.U", 	"titer av serumnøytraliserende antistoff fra vaksinasjon av marsvin");
        units.insert_or_assign("SQ-Bet", 	"Standardiserte kvalitetsenheter for bjørkepollen");
        units.insert_or_assign("SQ-E", 	"standardiserte kvalitetsenheter");
        units.insert_or_assign("SQ-HDM", 	"Standardized quality house dust mite");
        units.insert_or_assign("SQ-T", 	"standardiserte kvalitetsenheter for tablett");
        units.insert_or_assign("U", 	"unit");
        units.insert_or_assign("kIE", 	"Kilo internasjonale enheter");
        units.insert_or_assign("mEq", 	"milliekvivalent(er)");
        units.insert_or_assign("hgl", 	"Hetteglass");
    }
};

static const StrengthUnits strengthUnits{};

const UnitsOfMeasure &UnitsOfMeasure::GetUnitsForStrength() {
    return strengthUnits;
}
