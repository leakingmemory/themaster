//
// Created by sigsegv on 1/17/24.
//

#include "MedicalCodedValue.h"
#include <sfmbasisapi/fhir/value.h>

FhirCoding MedicalCodedValue::ToCoding() const {
    auto system = this->system;
    if (!system.empty() && !system.starts_with("urn:oid:") && !system.starts_with("http:")) {
        system.insert(0, "urn:oid:");
    }
    if (!shortDisplay.empty()) {
        FhirCoding coding{system, code, shortDisplay};
        return coding;
    } else {
        FhirCoding coding{system, code, display};
        return coding;
    }
}

FhirCodeableConcept MedicalCodedValue::ToCodeableConcept() const {
    if (code.empty()) {
        if (!display.empty()) {
            FhirCodeableConcept codeableConcept{display};
            return codeableConcept;
        } else if (!shortDisplay.empty()) {
            FhirCodeableConcept codeableConcept{shortDisplay};
            return codeableConcept;
        } else {
            return {};
        }
    }
    auto system = this->system;
    if (!system.empty() && !system.starts_with("urn:oid:") && !system.starts_with("http:")) {
        system.insert(0, "urn:oid:");
    }
    if (!shortDisplay.empty()) {
        FhirCodeableConcept codeableConcept{system, code, shortDisplay};
        return codeableConcept;
    } else {
        FhirCodeableConcept codeableConcept{system, code, display};
        return codeableConcept;
    }
}

template <const char * const system> class MedicalCodingSystem {
    friend MedicalCodedValue;
protected:
    std::vector<MedicalCodedValue> values{};
public:
    constexpr MedicalCodingSystem() {
    }
    constexpr void Add(const std::string &code, const std::string &display, const std::string &shortDisplay) {
        values.emplace_back(system, code, display, shortDisplay);
    }
    constexpr void Add(const std::string &code, const std::string &display) {
        values.emplace_back(system, code, display, display);
    }
};

constexpr const char medicamentForm[] = "urn:oid:2.16.578.1.12.4.1.1.7448";
constexpr const char recallCode[] = "urn:oid:2.16.578.1.12.4.1.1.7500";
constexpr const char cessationCode[] = "urn:oid:2.16.578.1.12.4.1.1.7494";
constexpr const char caveSourceOfInformation[] = "http://nhn.no/kj/fhir/CodeSystem/SourceOfInformation";
constexpr const char caveTypeOfReaction[] = "http://nhn.no/kj/fhir/CodeSystem/TypeOfReaction";
constexpr const char caveVerificationStatus[] = "http://terminology.hl7.org/CodeSystem/allergyintolerance-verification";

class VolvenMedicamentForm : public MedicalCodingSystem<medicamentForm> {
public:
    constexpr VolvenMedicamentForm();
};

class VolvenRecallCode : public MedicalCodingSystem<recallCode> {
public:
    constexpr VolvenRecallCode();
};

class VolvenCessationCode : public MedicalCodingSystem<cessationCode> {
public:
    constexpr VolvenCessationCode();
};

class CaveSourceOfInformation : public MedicalCodingSystem<caveSourceOfInformation> {
public:
    constexpr CaveSourceOfInformation();
};

class CaveTypeOfReaction : public MedicalCodingSystem<caveTypeOfReaction> {
public:
    constexpr CaveTypeOfReaction();
};

class CaveVerificationStatus : public MedicalCodingSystem<caveVerificationStatus> {
public:
    constexpr CaveVerificationStatus();
};

constexpr VolvenMedicamentForm::VolvenMedicamentForm() {
    Add("2", 	"Plaster til provokasjonstest", 	"plaster til provok test");
    Add("3", 	"Brusegranulat", 	"brusegranulat");
    Add("4", 	"Brusepulver", 	"brusepulv");
    Add("5", 	"Dispergerbar tablett", 	"disperg tab");
    Add("6", 	"Dråper", 	"dråper");
    Add("7", 	"Granulat", 	"gran");
    Add("9", 	"Mikstur", 	"mikst");
    Add("10", 	"Oppløselig tablett", 	"oppl tab");
    Add("11", 	"Oralgel", 	"oralgel");
    Add("12", 	"Oralpasta", 	"oralpasta");
    Add("14", 	"Sirup", 	"sirup");
    Add("15", 	"Urtete", 	"urtete");
    Add("16", 	"Brusetablett", 	"brusetab");
    Add("17", 	"Bukkaltablett", 	"bukkaltab");
    Add("18", 	"Dentalgel", 	"dentalgel");
    Add("20", 	"Dentalpulver", 	"dentalpulv");
    Add("21", 	"Dentalstift", 	"dentalstift");
    Add("22", 	"Dentalvæske", 	"dentalvæske");
    Add("23", 	"Depotgranulat", 	"depotgranulat");
    Add("24", 	"Depotkapsel", 	"depotkaps");
    Add("25", 	"Depottablett", 	"depottab");
    Add("26", 	"Enterogranulat", 	"enterogran");
    Add("28", 	"Enterotablett", 	"enterotab");
    Add("29", 	"Gel", 	"gel");
    Add("30", 	"Granuler", 	"granuler");
    Add("31", 	"Gurglevann", 	"gurglevann");
    Add("32", 	"Kapsel", 	"kaps");
    Add("33", 	"Konsentrat til gurglevann", 	"kons til gurglevann");
    Add("34", 	"Krem", 	"krem");
    Add("36", 	"Munngel", 	"munngel");
    Add("38", 	"Munnpasta", 	"munnpasta");
    Add("39", 	"Munnskyllevæske", 	"munnskyllevæske");
    Add("40", 	"Munnspray", 	"munnspray");
    Add("42", 	"Oblatkapsel", 	"oblatkaps");
    Add("43", 	"Oppløsning til tannkjøtt", 	"oppl til tannkjøtt");
    Add("44", 	"Pasta", 	"pasta");
    Add("46", 	"Salve", 	"salve");
    Add("47", 	"Skum", 	"skum");
    Add("48", 	"Smeltetablett", 	"smeltetab");
    Add("49", 	"Sublingvalspray", 	"sublingvalspray");
    Add("50", 	"Sublingvaltablett", 	"sublingvaltab");
    Add("51", 	"Sugepastill", 	"sugepastill");
    Add("52", 	"Sugetablett", 	"sugetab");
    Add("53", 	"Tablett", 	"tab");
    Add("54", 	"Tannpasta", 	"tannpasta");
    Add("55", 	"Tilsetning til badevann", 	"tilsetning til badevann");
    Add("56", 	"Medisinsk tyggegummi", 	"medisinsk tyggegummi");
    Add("57", 	"Tyggetablett", 	"tyggetab");
    Add("58", 	"Depotplaster", 	"depotplaster");
    Add("59", 	"Depotøyedråper", 	"depotøyedr");
    Add("61", 	"Hudstift", 	"hudstift");
    Add("62", 	"Impregnert pute", 	"impreg pute");
    Add("64", 	"Kollodium", 	"kollodium");
    Add("66", 	"Liniment", 	"liniment");
    Add("67", 	"Medisinsk neglelakk", 	"medisinsk neglelakk");
    Add("68", 	"Nesedråper", 	"nesedr");
    Add("69", 	"Nesegel", 	"nesegel");
    Add("70", 	"Nesekrem", 	"nesekrem");
    Add("71", 	"Nesepulver", 	"nesepulv");
    Add("72", 	"Nesesalve", 	"nesesalve");
    Add("73", 	"Neseskyllevæske", 	"neseskyllevæske");
    Add("74", 	"Nesespray", 	"nesespray");
    Add("75", 	"Nesestift", 	"nesestift");
    Add("76", 	"Omslag", 	"omslag");
    Add("77", 	"Pudder", 	"pudder");
    Add("78", 	"Salvekompress", 	"salvekompress");
    Add("79", 	"Sjampo", 	"sjampo");
    Add("80", 	"Øredråper", 	"ødredr");
    Add("81", 	"Øregel", 	"øregel");
    Add("87", 	"Ørestift", 	"ørestift");
    Add("88",	"Øretampong", 	"øretampong");
    Add("90", 	"Øyedråper", 	"øyedr");
    Add("91", 	"Øyegel", 	"øyegel");
    Add("92", 	"Øyekrem", 	"øyekrem");
    Add("93", 	"Øyelamell", 	"øyelamell");
    Add("94", 	"Øyesalve", 	"øyesalve");
    Add("95", 	"Infusjonsvæske", 	"inf væske");
    Add("97", 	"Væske til inhalasjonsdamp", 	"væske til inh damp");
    Add("98", 	"Inhalasjonsgass", 	"inh gass");
    Add("99", 	"Inhalasjonspulver", 	"inh pulv");
    Add("100", 	"Inhalasjonsvæske til nebulisator", 	"inh væske til nebulisator");
    Add("101", 	"Injeksjonsvæske", 	"inj væske");
    Add("102", 	"Pulver til infusjonsvæske", 	"pulv til inf væske");
    Add("103", 	"Pulver til injeksjonsvæske", 	"pulv til inj væske");
    Add("104", 	"Rektalgel", 	"rektalgel");
    Add("105", 	"Rektalkapsel", 	"rektalkaps");
    Add("106", 	"Rektalkrem", 	"rektalkrem");
    Add("107", 	"Rektalsalve", 	"rektalsalve");
    Add("108", 	"Rektalskum", 	"rektalskum");
    Add("109", 	"Rektaltampong", 	"rektaltampong");
    Add("110", 	"Rektalvæske", 	"rektalvæske");
    Add("112", 	"Stikkpille", 	"stikkpille");
    Add("113", 	"Vaginalbrusetablett", 	"vag brusetab");
    Add("114", 	"Vaginalgel", 	"vag gel");
    Add("115", 	"Vaginalinnlegg", 	"vag innlegg");
    Add("117", 	"Vaginalkrem", 	"vag krem");
    Add("118", 	"Vaginalsalve", 	"vag salve");
    Add("119", 	"Vaginalskum", 	"vag skum");
    Add("120", 	"Vaginaltablett", 	"vag tab");
    Add("122", 	"Vaginalvæske", 	"vag væske");
    Add("123", 	"Vagitorie", 	"vagitorie");
    Add("124", 	"Blæreskyllevæske", 	"blæreskyllevæske");
    Add("125", 	"Endocervikalgel", 	"endocervikalgel");
    Add("126", 	"Endotrakeopulmonal instillasjonsvæske", 	"endotrakeopulml instill væske");
    Add("128", 	"Hemodiafiltreringsvæske", 	"hemodiafilt væske");
    Add("129", 	"Hemodialysekonsentrat", 	"hemodial kons");
    Add("130", 	"Hemodialysevæske", 	"hemodial væske");
    Add("131", 	"Hemofiltreringsvæske", 	"hemofilt væske");
    Add("132", 	"Implantat" ,	"implantat");
    Add("133", 	"Intrauterint innlegg", 	"intrauterint innl");
    Add("134", 	"Kjede til implantasjon", 	"kjede til impl");
    Add("135", 	"Mageskyllevæske", 	"mageskyllevæske");
    Add("136", 	"Oppløsning", 	"oppl");
    Add("138", 	"Peritonealdialysevæske", 	"peritonealdialysevæske");
    Add("139", 	"Pulver", 	"pulv");
    Add("140", 	"Radionuklidegenerator", 	"radionuklidegenerator");
    Add("142", 	"Skyllevæske", 	"skyllevæske");
    Add("144", 	"Sårstift", 	"sårstift");
    Add("146", 	"Urethralgel", 	"urethralgel");
    Add("147", 	"Urethralstift", 	"urethralstift");
    Add("148", 	"Vevslim", 	"vevslim");
    Add("152", 	"Spray", 	"spray");
    Add("153", 	"Oppløsning til prikktest", 	"oppl til prikktest");
    Add("159", 	"Kombinasjonspakning", 	"Kombinasjonspakning");
    Add("160", 	"Inhalasjonsvæske", 	"inh væske");
    Add("163", 	"Dentallakk", 	"dentallakk");
    Add("165", 	"Påflekkingsvæske", 	"påflekkingsvæske");
    Add("170", 	"Konsentrat til bad", 	"kons til bad");
    Add("171", 	"Konsentrat til mikstur", 	"kons til mikst");
    Add("174", 	"Medisinert halsbånd", 	"medisinert halsbånd");
    Add("176", 	"Levende vevserstatning", 	"levende vevserstatning");
    Add("177", 	"Medisinert plaster", 	"medisinert plaster");
    Add("178", 	"Medisinpellet" ,	"medisinpellet");
    Add("180", 	"Pulver til injeksjons-/infusjonsvæske", 	"pulv til inj/inf væske");
    Add("181", 	"Ørebrikke", 	"ørebrikke");
    Add("184", 	"Intravesikaloppløsning", 	"intravesikaloppl");
    Add("187", 	"Påhellingsvæske", 	"påhellingsvæske");
    Add("193", 	"Impregnert kompress", 	"impreg kompress");
    Add("196", 	"Enterodepotgranulat", 	"enterodepotgran");
    Add("199", 	"Depotgranulat til mikstur, suspensjon", 	"depotgranulat til mikst");
    Add("200", 	"Suspensjon til implantasjon", 	"susp til impl");
    Add("201", 	"Injeksjonsvæske, SurePal 10", 	"inj væske, surepal 10");
    Add("202", 	"Injeksjonsvæske, SurePal 5", 	"inj væske, surepal 5");
    Add("206", 	"Pulver til sirup", 	"pulv til sirup");
    Add("208", 	"Oral oppløsning", 	"oral oppl");
    Add("209", 	"Periodontalgel", 	"periodontalgel");
    Add("212", 	"Medisinert svamp", 	"medisinert svamp");
    Add("213", 	"Intestinalgel", 	"intestinalgel");
    Add("216", 	"Oral emulsjon", 	"oral emulsjon");
    Add("217", 	"Transdermalt system", 	"transdermalt system");
    Add("218", 	"Oralt pulver", 	"oralt pulv");
    Add("219", 	"Medisinsk gass, kryogen", 	"medisinsk gass, kryogen");
    Add("220", 	"Intramammarie, salve", 	"intramam, salve");
    Add("223", 	"Pulver til mikstur/rektalvæske, suspensjon", 	"pulv til mikst/rektalvæske, susp");
    Add("224", 	"Pulver til mikstur, suspensjon", 	"pulv til mikst, susp");
    Add("225", 	"Enterodepottablett", 	"enterodepottab");
    Add("227", 	"Bikubestrip", 	"bikubestrip");
    Add("229", 	"Gel til injeksjon", 	"inj gel");
    Add("232", 	"Medisinsk anheng", 	"medisinsk anheng");
    Add("233", 	"Nebulisasjonsoppløsning", 	"nebulisasjonsoppl");
    Add("234", 	"Pastill", 	"pastill");
    Add("235", 	"Periodontalinnlegg", 	"periodontalinnlegg");
    Add("237", 	"Periodontalpulver", 	"periodontalpulv");
    Add("238", 	"Røykpapir til bier", 	"røykpapir til bier");
    Add("239", 	"Røykstift til bier", 	"røykstift til bier");
    Add("240", 	"Slikkestein", 	"slikkestein");
    Add("241", 	"Medisinert tråd", 	"medisinert tråd");
    Add("243", 	"Spenespray, oppløsning", 	"spenespray, oppl");
    Add("244", 	"Spenestift", 	"spenestift");
    Add("245", 	"Tannproteselakk", 	"tannproteselakk");
    Add("246", 	"Tyggekapsel, myk", 	"tyggekaps");
    Add("248", 	"Oppløsning til provokasjonstest", 	"oppl til provokasjonstest");
    Add("252", 	"Oppløsning til risptest", 	"oppl til risptest");
    Add("276", 	"Dentaltampong", 	"dentaltampong");
    Add("307", 	"Tablett til implantasjon", 	"tab til impl");
    Add("320", 	"Injeksjonsvæske/konsentrat til infusjonsvæske", 	"inj væske/kons til inf væske");
    Add("329", 	"Konsentrat til bruk i drikkevann", 	"kons til bruk i drikkevann");
    Add("341", 	"Oppløsning til oral og rektal bruk", 	"oppl til oral og rektal bruk");
    Add("354", 	"Premiks til medisinert fôr", 	"premiks til medisinert fôr");
    Add("398", 	"Enterogranulat til mikstur, suspensjon", 	"enterogran til mikst");
    Add("399", 	"Gel til tannkjøtt", 	"gel til tannkjøtt");
    Add("400", 	"Granulat til dråper", 	"gran til dråper");
    Add("401", 	"Granulat til sirup", 	"gran til sirup");
    Add("403", 	"Oppløsning til blodfraksjonsmodifisering", 	"oppl til blodfraksjonsmodifisering");
    Add("404", 	"Oppløsningsvæske til parenteral bruk", 	"oppl væske til parenteral bruk");
    Add("405", 	"Tablett til gurglevann, oppløsning", 	"tab til gurglevann, oppl");
    Add("407", 	"Hemodialyse-/hemofiltreringsvæske", 	"hemodial/hemofilt væske");
    Add("409", 	"Injeksjons-/infusjonsvæske", 	"inj/inf væske");
    Add("413", 	"Pulver til bad, oppløsning", 	"pulv til bad, oppl");
    Add("414", 	"Pulver til blæreskyllevæske", 	"pulv til blæreskyll");
    Add("415", 	"Pulver til bruk i drikkevann", 	"pulv til bruk i drikkevann");
    Add("418", 	"Tablett til rektalvæske", 	"tab til rektalvæske");
    Add("419", 	"Lyofilisat til injeksjonsvæske", 	"lyofil til inj væske");
    Add("421", 	"Lyofilisat til infusjonsvæske" ,	"lyofil til inf væske");
    Add("423", 	"Stamoppløsning til radioaktive legemidler", 	"stamoppl til radioaktive legemidler");
    Add("424", 	"Inhalasjonspulver, hard kapsel", 	"inh pulv, kaps");
    Add("425", 	"Granulat i dosepose", 	"gran i dosepose");
    Add("426", 	"Injeksjonsvæske, suspensjon", 	"inj væske, susp");
    Add("427", 	"Pulver til injeksjonsvæske/intravesikaloppløsning", 	"pulv til inj væske/intravesikaloppl");
    Add("428", 	"Resoriblett, slimhinneklebende", 	"resoriblett, slimhinneklebende");
    Add("429", 	"Preparasjonssett til radioaktive legemidler", 	"prep sett til radioaktive legemidler");
    Add("430", 	"Mikstur/rektalvæske, oppløsning", 	"mikst/rektalvæske, oppl");
    Add("431", 	"Pulver og suspensjon til injeksjonsvæske", 	"pulv og susp til inj væske");
    Add("433", 	"Pulver og væske til infusjonsvæske", 	"pulv+væske til inf væske");
    Add("435", 	"Pulver og væske til injeksjonsvæske", 	"pulv+væske til inj væske");
    Add("437", 	"Pulver og væske til mikstur, suspensjon", 	"pulv+væske til mikst susp");
    Add("438", 	"Pulver og væske til intravesikaloppløsning", 	"pulv og væske til intravesikaloppl");
    Add("439", 	"Pulver og væske til intravesikalsuspensjon", 	"pulv og væske til intravesikalsusp");
    Add("440", 	"Pulver og væske til suspensjon til nasal bruk", 	"pulv og væske til susp til nasal bruk");
    Add("441", 	"Oppløsning til vevslim", 	"oppl til vevslim");
    Add("442", 	"Pulver og væske til vevslim", 	"pulv og væske til vevslim");
    Add("443", 	"Pulver til konsentrat til infusjonsvæske", 	"pulv til kons til inf væske");
    Add("444", 	"Pulver og væske til konsentrat til infusjonsvæske, oppløsning", 	"pulv+væske til kons til inf væske oppl");
    Add("445", 	"Pulver til suspensjon til implantasjon", 	"pulv til susp til implantasjon");
    Add("446", 	"Pulver til intravesikalsuspensjon" ,	"pulv til intravesikalsusp");
    Add("447", 	"Granulat med modifisert frisetting", 	"gran modif frisetting");
    Add("449", 	"Tablett med modifisert frisetting", 	"tab modif frisetting");
    Add("451", 	"Suspensjon og brusegranulat til mikstur, suspensjon", 	"susp+brusegran til mikst, susp");
    Add("453", 	"Intravitrealt implantat", 	"intravitrealt implantat");
    Add("454", 	"Injeksjonsvæske til intraartikulær bruk", 	"inj væske til intraartikulær bruk");
    Add("456", 	"Intraruminalinnlegg, periodevis frisetting", 	"intraruminalinnl periode frisett");
    Add("457", 	"Lyofilisat og væske til injeksjonsvæske", 	"lyofil og væske til inj væske");
    Add("458", 	"Lyofilisat og væske til nasal bruk, suspensjon", 	"lyofil+væske til nasal bruk, susp");
    Add("460", 	"Konsentrat til infusjonsvæske", 	"kons til inf væske");
    Add("461", 	"Konsentrat til injeksjons-/infusjonsvæske, oppløsning", 	"kons til inj/inf oppl");
    Add("462", 	"Konsentrat og væske til infusjonsvæske, oppløsning", 	"kons+væske til inf oppl");
    Add("464", 	"Uteritorie, tablett", 	"uteritorie, tab");
    Add("465", 	"Uteritorie, kapsel", 	"uteritorie, kaps");
    Add("466", 	"Konsentrat til behandlingsoppløsning til fisk", 	"kons til behandlingsoppl");
    Add("468", 	"Munnsalve", 	"munnsalve");
    Add("469", 	"Bikubegel", 	"bikubegel");
    Add("470", 	"Lyofilisat og væske til mikstur, suspensjon", 	"lyofil+væske til mikst susp");
    Add("472" ,	"Munnsmeltende film", 	"munnsmeltende film");
    Add("476", 	"Bukkalfilm", 	"bukkalfilm");
    Add("478", 	"Liniment, oppløsning", 	"liniment, oppl");
    Add("479", 	"Liniment, suspensjon", 	"liniment, susp");
    Add("480", 	"Liniment, emulsjon", 	"liniment, emulsjon");
    Add("484", 	"Sublingvalfilm", 	"sublingvalfilm");
    Add("485", 	"Oppløsning til hodebunnen", 	"oppl til hodebunn");
    Add("487", 	"Konsentrat og væske til konsentrat til infusjonsvæske, oppløsning", 	"Kons og væske til kons til inf væske, oppl");
    Add("489", 	"Transdermaloppløsning", 	"transdermaloppl");
    Add("490", 	"Lyofilisat og væske til injeksjons-/infusjonsvæske, oppløsning", 	"lyofil og væske til inj/inf væske, oppl");
    Add("491", 	"Suspensjon til bruk i drikkevann", 	"susp til bruk i drikkevann");
    Add("492", 	"Medisinert serviett", 	"medisinert serviett");
    Add("493", 	"Injeksjons-/infusjonsvæske/rektalvæske", 	"inj/inf væske el rektalvæske");
    Add("494", 	"Konsentrat til bad/injeksjonsvæske, suspensjon", 	"kons til bad/inj susp");
    Add("496", 	"Lyofilisat og oppløsningsvæske til suspensjon", 	"lyofil og væske til susp");
    Add("497" ,	"Munnpulver", 	"munnpulv");
    Add("498", 	"Konsentrat og væske til injeksjons-/infusjonsvæske, oppløsning", 	"kons+væske til inj/inf oppl");
    Add("501", 	"Pulver og gel til gel", 	"pulv og gel til gel");
    Add("502", 	"Pulver og væske til intraokulær instillasjonsvæske, oppløsning", 	"pulv+væske til intraokul inst væske oppl");
    Add("503", 	"Øyestrip", 	"øyestrip");
    Add("504", 	"Pulver til intravesikaloppløsning/injeksjons-/infusjonsvæske", 	"pulv til intravesikaloppl/inj/inf væske");
    Add("505", 	"Pulver og væske til inhalasjonsvæske til nebulisator, oppløsning", 	"pulv+væske til inh væske oppl");
    Add("506", 	"Suspensjon og væske til injeksjonsvæske, suspensjon", 	"susp+væske til inj væske, susp");
    Add("507", 	"Matriks til vevslim", 	"matriks til vevslim");
    Add("508", 	"Matriks til implantasjon", 	"matriks til implantasjon");
    Add("509", 	"Lyofilisat til bruk i drikkevann", 	"lyofil til drikkevann");
    Add("510", 	"Oppløsning til bruk i drikkevann", 	"oppl til bruk i drikkevann");
    Add("511", 	"Pulver til behandlingssuspensjon til fisk", 	"pulv til behandlingssusp");
    Add("512", 	"Hudplaster", 	"plaster");
    Add("513", 	"Transdermalgel", 	"transdermalgel");
    Add("514", 	"Gass og væske til injeksjons-/infusjonsvæske, dispersjon", 	"gass+væske til inj/inf");
    Add("517", 	"Infusjonsvæske og mikstur, oppløsning", 	"inf væske og mikst, oppl");
    Add("518", 	"Injeksjonsvæske, SurePal 15", 	"inj væske, surepal 15");
    Add("519", 	"Lyofilisat til mikstur, suspensjon", 	"lyofil til mikst, susp");
    Add("520", 	"Pulver til injeksjons-/infusjons- eller installasjonsvæske, oppløsning", 	"pulv til inj/inf/inst væske oppl");
    Add("521", 	"Øredråper/liniment, suspensjon", 	"øredr/liniment, susp");
    Add("522", 	"Pulver og væske til liniment, oppløsning", 	"pulv+væske til lin oppl");
    Add("523", 	"Konsentrat til intraokulær skyllevæske, oppløsning", 	"kons til intraokulær skyll oppl");
    Add("524", 	"Tyggetablett/dispergerbar tablett", 	"tyggetab/disperg tab");
    Add("526", 	"Transdermalspray, oppløsning", 	"transdermalspray, oppl");
    Add("527", 	"Lyofilisat og suspensjon til injeksjonsvæske, suspensjon", 	"lyofil og susp til inj væske");
    Add("528", 	"Konsentrat og væske til injeksjonsvæske", 	"kons og væske til inj væske");
    Add("529", 	"Gelepute", 	"gelepute");
    Add("530", 	"Øre-/øye-/nesedråper, oppløsning", 	"øre/øye/nesedråper, oppl");
    Add("531", 	"Medisinsk blodigle", 	"medical leech");
    Add("533", 	"Øredråper, oppløsning", 	"øredr, oppl");
    Add("534", 	"Suspensjon og oppløsning til spray", 	"susp og oppl til spray");
    Add("535", 	"Vaginal", 	"vaginal");
    Add("536", 	"Urethralemulsjon", 	"urethral emul");
    Add("539", 	"Intestinal bruk", 	"intestinal bruk");
    Add("540", 	"Pulver til depotinjeksjonsvæske, suspensjon", 	"pulv til depotinj susp");
    Add("541", 	"Pulver, væske og matriks til matriks til implantasjon", 	"pulv, væske og matriks til matriks til impl");
    Add("543", 	"Pulver til bikubeoppløsning", 	"pulv til bikubeoppl");
    Add("544", 	"Transdermalsalve", 	"transdermalsalve");
    Add("546", 	"Oppløsning til bikubedispersjon" ,	"oppl til bikubedisp");
    Add("547", 	"Suspensjon til mikstur, suspensjon", 	"susp til mikst, susp");
    Add("548", 	"Pulver til behandlingsoppløsning til fisk", 	"pulv til beh.oppl til fisk");
    Add("549", 	"Bikubedispersjon", 	"bikubedisp");
    Add("550", 	"Intrauteringel", 	"intrauteringel");
    Add("551", 	"Oppløsning til organoppbevaring", 	"oppl til organoppbevaring");
    Add("552", 	"Oppløsning til kardioplegi", 	"oppl til kardioplegi");
    Add("553", 	"Oppløsning til kardioplegi/organoppbevaring", 	"oppl til kardioplegi/organoppbevaring");
    Add("555", 	"Lyofilisat til nesespray, suspensjon", 	"lyofil til nesespray susp");
    Add("557", 	"Lyofilisat til okulonasalsuspensjon/bruk i drikkevann", 	"lyofil til okulonasalsusp/bruk i drikkevann");
    Add("560", 	"Lyofilisat til oralspray, suspensjon", 	"lyofil til oralspray susp");
    Add("561", 	"Konsentrat til konsentrat til oralspray, suspensjon", 	"kons til oralspray susp");
    Add("562", 	"Oralspray, suspensjon", 	"oralspray susp");
    Add("563", 	"Oralt plantemateriale", 	"oralt plantemateriale");
    Add("564", 	"Laryngofaryngeal spray, oppløsning", 	"laryngofaryngeal spray oppl");
    Add("565", 	"Tablett med sensor", 	"tab med sensor");
    Add("566", 	"Bikubeoppløsning", 	"bikubeoppl");
    Add("567", 	"Granulat i kapsler som åpnes", 	"gran som åpnes");
    Add("568", 	"Injeksjonsvæske, SoloStar", 	"inj væske, solostar");
    Add("569", 	"Injeksjonsvæske, DoubleStar", 	"inj væske, doublestar");
    Add("570", 	"Lyofilisat og væske til injeksjon eller nesespray, suspensjon", 	"lyofil+væske til inj/nesespray susp");
    Add("571", 	"Tablett og væske til rektalvæske, suspensjon", 	"tab og væske til rektalvæske, susp");
    Add("572", 	"Granulat, drasjert", 	"granulat, drasjert");
    Add("573", 	"Injeksjonsvæske, injektor", 	"inj væske, injektor");
    Add("574", 	"Pulver, dispersjon og væske til konsentrat til infusjonsvæske, dispersjon", 	"pulv, disper og væske til kons til inf væske");
    Add("579", 	"Lyofilisat og væske til nesedråper, suspensjon", 	"lyofil+væske til nesedr, susp");
    Add("580", 	"Lyofilisat og væske til nesespray, suspensjon", 	"lyofil+væske til nesespay susp");
    Add("581", 	"Pulver og væske til gel til tannkjøtt", 	"pulv+væske til tannkjøttgel");
    Add("582", 	"Pulver og væske til oppløsning skadet område", 	"pulv+væske til oppl til skade");
    Add("583", 	"Pulver og oppløsning til injeksjonsvæske", 	"pulv+oppl til inj");
    Add("584", 	"Pulver og oppløsning til dentalsement", 	"pulv+oppl til dentalsement");
    Add("585", 	"Tablett og pulver til mikstur, oppløsning", 	"tab + pulv til mikst, oppl");
    Add("586", 	"Pulver til konsentrat og oppløsning til infusjonsvæske, oppløsning", 	"pulv til kons+oppl til inf");
    Add("587", 	"Emulsjon og suspensjon til injeksjonsvæske, emulsjon", 	"emul+susp til inj væske, emul");
    Add("588", 	"Konsentrat til hudspray, emulsjon", 	"kons til hudspray, emul");
    Add("589", 	"Konsentrat til mikstur/rektalvæske, oppløsning", 	"kons til mikst/rektalvæske oppl");
    Add("590", 	"Dentalpasta", 	"dentalpasta");
    Add("591", 	"Øre-/øyesalve", 	"øre/øyesalve");
    Add("592", 	"Bihuleskyllevæske, suspensjon", 	"bihuleskyll, susp");
    Add("593", 	"Gurglevann/munnskyllevæske", 	"gurglevann/munnskyllevæske");
    Add("594", 	"Granulat til vaginalvæske, oppløsning", 	"gran til vag væske, oppl");
    Add("595", 	"Brusetablett til inhalasjonsdamp", 	"brusetab til inh damp");
    Add("596", 	"Impregnert pute til inhalasjonsdamp", 	"impregnert pute til inh damp");
    Add("597", 	"Mikstur, oppløsning/konsentrat til inhalasjonsvæske til nebulisator, oppløsning", 	"mikst oppl/kons til inh væske til nebulisator, oppl");
    Add("598", 	"Inhalasjonspulver, tablett", 	"inh pulv, tab");
    Add("599", 	"Emulsjon til inhalasjonsdamp", 	"emulsjon til inh damp");
    Add("600", 	"Granulat med modifisert frisetting til mikstur, suspensjon", 	"gran modif frisetting til mikst,suspensjon");
    Add("601", 	"Munnplaster", 	"munnplaster");
    Add("602", 	"Pulver til intravesikaloppløsning", 	"pulv til intravesikaloppl");
    Add("603", 	"Pulver til intraokulær skyllevæske, oppløsning", 	"pulv til intraokulær skyllevæske, oppl");
    Add("604", 	"Oppløsningsvæske til infusjonsvæske, oppløsning", 	"oppl væske til inf, oppl");
    Add("605", 	"Pulver til dråper, suspensjon", 	"pulv til dråper, susp");
    Add("606", 	"Oppløsningsvæske til intraokulær skyllevæske, oppløsning", 	"oppl til intraokulær skyllevæske, oppl");
    Add("607", 	"Granulat til bruk i drikkevann", 	"granulat til drikkevann");
    Add("608", 	"Intraperitonealoppløsning", 	"intraperitonealoppl");
    Add("609", 	"Konsentrat til intravesikaloppløsning", 	"kons til intravesikaloppl");
    Add("610", 	"Konsentrat til peritonealdialysevæske", 	"kons til peritonealdialysevæske");
    Add("611", 	"Konsentrat til spray, emulsjon", 	"kons til spray, emul");
    Add("612", 	"Hud-/nesesalve", 	"hud-/nesesalve");
    Add("613", 	"Intrauterinskum", 	"intrauterinskum");
    Add("614", 	"Nesedråper/munnvann, oppløsning", 	"nesedr/munnvann,oppl");
    Add("615", 	"Nese-/munnspray, oppløsning", 	"nese-/munnspray,oppl");
    Add("616", 	"Nesespray/munnvann, oppløsning", 	"nesespray/munnvann,oppl");
    Add("617", 	"Munnvann, oppløsning/laryngofaryngeal oppløsning", 	"munnvann,oppl/laryngofaryngeal oppl");
    Add("618", 	"Munnvann/laryngofaryngeal oppløsning/spray, oppløsning", 	"munnvann/laryngofaryngeal oppl/spray, oppl");
    Add("619", 	"Øre-/øyedråper, suspensjon", 	"øre/øyedr, susp");
    Add("620", 	"Dentalsement", 	"dentalsement");
    Add("621", 	"Øre-/nesedråper, suspensjon", 	"øre/nesedr, susp");
    Add("622", 	"Pulver til vevslim", 	"pulv til vevslim");
    Add("623", 	"Lyofilisat til nesedråper, suspensjon", 	"lyofil til nesedr, susp");
    Add("624", 	"Pulver til oppløsning til skadet område", 	"pulv til oppl til skadet område");
    Add("625", 	"Intraokulær instillasjonsvæske, oppløsning", 	"intraokulær inst væske oppl");
    Add("626", 	"Intraokulær skyllevæske, oppløsning", 	"intraokulær skyllevæske, oppl");
    Add("627", 	"Intravesikalsuspensjon", 	"intravesikalsusp");
    Add("628", 	"Okulonasalsuspensjon", 	"okulonasalsusp");
    Add("629", 	"Pulver til iontoforeseoppløsning", 	"pulv til iontoforeseoppl");
    Add("630", 	"Oppløsningsvæske til…", 	"oppl væske til...");
    Add("631", 	"Bihulevæske, oppløsning", 	"bihulevæske, oppl");
    Add("632", 	"Brusegranulat til mikstur, suspensjon", 	"brusegranulat til mikst");
    Add("633", 	"Dispersjon til konsentrat til infusjonsvæske, dispersjon", 	"disp til kons til inf");
    Add("634", 	"Emulsjon til injeksjonsvæske, suspensjon", 	"emul til inj, susp");
    Add("635", 	"Emulsjon til injeksjonsvæske, emulsjon", 	"emul til inj, emul");
    Add("636", 	"Gass til infusjonsvæske, dispersjon", 	"gass til inf væske, dispersjon");
    Add("637", 	"Gass til injeksjons-/infusjonsvæske, dispersjon", 	"gss til inj-/inf væske, dispersjon");
    Add("638", 	"Gass til injeksjonsvæske, dispersjon", 	"gass til inj");
    Add("639", 	"Gel til gel", 	"gel til gel");
    Add("640", 	"Granulat til injeksjonsvæske, suspensjon", 	"granulat til inj");
    Add("641", 	"Granulat til mikstur/rektalvæske, suspensjon", 	"gran til mikst/rektalvæske, susp");
    Add("642", 	"Granulat til rektalvæske, suspensjon", 	"gran til rektalvæske,susp");
    Add("643", 	"Gurglevann/neseskyllevæske", 	"gurglevann/neseskyllevæske");
    Add("644", 	"Impregnert plugg", 	"impreg plugg");
    Add("645", 	"Impregnert plugg til inhalasjonsdamp", 	"impregnert plugg til inh damp");
    Add("646", 	"Injeksjons-/infusjonsvæske, dispersjon", 	"inj/inf, disp");
    Add("647", 	"Intramammarie, gel", 	"intramam, gel");
    Add("648", 	"Konsentrat til inhalasjonsvæske til nebulisator, oppløsning", 	"kons til inh væske oppl");
    Add("649", 	"Konsentrat til injeksjonsvæske, suspensjon", 	"kons til inj susp");
    Add("650", 	"Konsentrat til konsentrat til infusjonsvæske, oppløsning", 	"kons til kons til inf væske, oppl");
    Add("651", 	"Konsentrat til munnvann, oppløsning", 	"kons til munnvann,oppl");
    Add("652", 	"Laryngofaryngeal oppløsning", 	"laryngofaryngeal oppl");
    Add("653", 	"Liniment, oppløsning/konsentrat til munnvann, oppløsning", 	"liniment,oppl/kons til munnvann");
    Add("654", 	"Matriks til matriks til implantasjon", 	"matriks til matriks til implantasjon");
    Add("655", 	"Oppløsning til bikubeoppløsning", 	"oppl til bikubeoppl");
    Add("656", 	"Oppløsning til bikubestrip", 	"oppl til bikubestrip");
    Add("657", 	"Oppløsning til bruk i drikkevann/melk", 	"oppl til bruk i drikkevann/melk");
    Add("658", 	"Oppløsning til dentalsement", 	"oppl til dentalsement");
    Add("659", 	"Pulver til dentalsement", 	"pulv til dentalsement");
    Add("660", 	"Oppløsning til infusjonsvæske, oppløsning", 	"oppl til inf væske");
    Add("662", 	"Oppløsning til injeksjonsvæske, suspensjon", 	"oppl til inj væske, susp");
    Add("663", 	"Oppløsning til injeksjonsvæske, oppløsning", 	"oppl til inj væske, oppl");
    Add("664", 	"Oppløsning til skadet område", 	"oppl til skadet område");
    Add("665", 	"Pasta til implantasjon", 	"pasta til implantasjon");
    Add("666", 	"Pulver til bikubedispersjon", 	"pulv til bikubedispersjon");
    Add("667", 	"Pulver til dentalgel", 	"pulv til dentalgel");
    Add("668", 	"Pulver til dentalvæske, oppløsning", 	"pulv til dentalvæske, oppl");
    Add("669", 	"Pulver til endocervikalgel", 	"pulv til endocervikalgel");
    Add("670", 	"Pulver til gel til tannkjøtt", 	"pulv til gel til tannkjøtt");
    Add("671", 	"Pulver til hemodialysekonsentrat", 	"pulv til hemodial kons");
    Add("672", 	"Pulver til injeksjonsvæske, dispersjon", 	"pulv til inj væske, dispersjon");
    Add("673", 	"Pulver til injeksjonsvæske, emulsjon", 	"pulv til inj væske, emulsjon");
    Add("674", 	"Pulver til bihulevæske, oppløsning", 	"pulv til bihulevæske, oppl");
    Add("675", 	"Pulver til bruk i drikkevann/melk", 	"pulv til bruk i drikkevann/melk");
    Add("676", 	"Pulver til intraokulær instillasjonsvæske, oppløsning", 	"pulv til intraokulær instill væske, oppl");
    Add("677", 	"Pulver til liniment, oppløsning", 	"pulv til liniment, oppl");
    Add("678", 	"Tablett til liniment, oppløsning", 	"tab til liniment, oppl");
    Add("679", 	"Pulver til konsentrat til intravesikalsuspensjon", 	"pulv til kons til intravesikalsusp");
    Add("680", 	"Pulver til injeksjonsvæske/prikktest, oppløsning", 	"pulv til inj væske/prikktest, oppl");
    Add("681", 	"Pulver og væske til injeksjonsvæske/prikktest, oppløsning", 	"pulv+væske til inj/prikktest, oppl");
    Add("682", 	"Pulver til prikktest, oppløsning", 	"pulv til prikktest, oppl");
    Add("683", 	"Pulver til matriks til implantasjon", 	"pulver til matriks til implantasjon");
    Add("684", 	"Pulver til pasta til implantasjon", 	"pulv til pasta til implantasjon");
    Add("685", 	"Pulver til munnskyllevæske, oppløsning", 	"pulv til munnskyllevæske, oppl");
    Add("686", 	"Pulver til nesedråper, oppløsning", 	"pulv til nesedr, oppl");
    Add("687", 	"Pulver til øredråper, suspensjon", 	"pulv til øredråper, susp");
    Add("688", 	"Pulver til nesespray, oppløsning", 	"pulv til nesespr, oppl");
    Add("689", 	"Pulver til øyedråper, suspensjon", 	"pulv til øyedr, susp");
    Add("690", 	"Pulver til øyedråper, oppløsning", 	"pulv til øyedr, oppl");
    Add("691", 	"Spenedypp/spenespray, oppløsning", 	"spenedypp/spenespray,oppl");
    Add("692", 	"Sublingvalpulver", 	"sublingvalpulv");
    Add("694", 	"Suspensjon til injeksjonsvæske, suspensjon", 	"susp til inj væske, susp");
    Add("695", 	"Suspensjon til injeksjonsvæske, emulsjon", 	"susp til inj væske, emul");
    Add("696", 	"Lyofilisat til injeksjonsvæske, emulsjon", 	"lyofil til inj væske, emulsjon");
    Add("697", 	"Lyofilisat til suspensjon til sprayvaksinering eller til bruk i drikkevann", 	"lyofil til susp til sprayvaksinering eller til drikkevann");
    Add("698", 	"Depotsåroppløsning", 	"depotsåroppl");
    Add("699", 	"Intramammarie, emulsjon", 	"intramam, emul");
    Add("700", 	"Intramammarie, oppløsning", 	"intramam, oppl");
    Add("701", 	"Intramammarie, suspensjon", 	"intramam, susp");
    Add("702", 	"Pulver i kapsler som åpnes", 	"pulv i kaps som åpnes");
    Add("703", 	"Depotinjeksjonsvæske, dispersjon", 	"depotinj væske, disp");
    Add("704", 	"Konsentrat til injeksjonsvæske, dispersjon", 	"kons til inj disp");
    Add("705", 	"Antikoagulasjons- og konserveringsoppløsning for blod", 	"antikoag/konserv oppl for blod");
    Add("706", 	"Pulver til injeksjons-/infusjons-/inhalasjonsvæske, oppløsning", 	"pulv til inj/inf/inhal væske, oppl");
    Add("708", 	"Bad, emulsjon", 	"bad, emul");
    Add("709", 	"Salve til inhalasjonsdamp", 	"salve til inh damp");
    Add("710", 	"Ørespray, emulsjon", 	"ørespray, emul");
    Add("711", 	"Øredråper, suspensjon", 	"øredr, susp");
    Add("712", 	"Bad, oppløsning", 	"bad, oppl");
    Add("713", 	"Øredråper, emulsjon", 	"øredr, emul");
    Add("714", 	"Øreskyllevæske, emulsjon", 	"øreskyll, emul");
    Add("715", 	"Ørespray, oppløsning", 	"ørespray, oppl");
    Add("716", 	"Øreskyllevæske, oppløsning", 	"øreskyll, oppl");
    Add("717", 	"Ørespray, suspensjon", 	"ørespray, susp");
    Add("718", 	"Bad, suspensjon", 	"bad, susp");
    Add("719", 	"Øyedråper, emulsjon", 	"øyedr, emul");
    Add("720", 	"Øyedråper, oppløsning", 	"øyedr, oppl");
    Add("721", 	"Øyedråper, suspensjon", 	"øyedr, susp");
    Add("722", 	"Vaginalkapsel, hard", 	"vag kaps");
    Add("723", 	"Vaginalkapsel, myk", 	"vag kaps");
    Add("724", 	"Vaginalvæske, emulsjon", 	"vag væske, emul");
    Add("725", 	"Vaginalvæske, oppløsning", 	"vag væske, oppl");
    Add("726", 	"Vaginalvæske, suspensjon", 	"vag væske, susp");
    Add("727", 	"Tablett til munnskyllevæske, oppløsning", 	"tab til munnskyll, oppl");
    Add("728", 	"Tablett til vaginalvæske, oppløsning", 	"tab til vag væske, oppl");
    Add("729", 	"Tablett til rektalvæske, oppløsning", 	"tab til rektalvæske, oppl");
    Add("730", 	"Tablett til rektalvæske, suspensjon", 	"tab til rektalvæske, susp");
    Add("731", 	"Tablett til inhalasjonsdamp", 	"tab til inh damp");
    Add("732", 	"Dentalvæske, emulsjon", 	"dentalvæske, emul");
    Add("733", 	"Dentalvæske, oppløsning", 	"dentalvæske, oppl");
    Add("734", 	"Dentalvæske, suspensjon", 	"dentalvæske, susp");
    Add("736", 	"Sublingvalspray, emulsjon", 	"sublingvalspray, emul");
    Add("737", 	"Sublingvalspray, oppløsning", 	"sublingvalspray, oppl");
    Add("738", 	"Sublingvalspray, suspensjon", 	"sublingvalspray, susp");
    Add("739", 	"Stamoppløsning til radioaktive legemidler, oppløsning", 	"stamoppl til radioaktivt legemiddel, oppl");
    Add("740", 	"Spenedypp, emulsjon" ,	"spenedypp, emul");
    Add("741", 	"Spenedypp, oppløsning", 	"spenedypp, oppl");
    Add("742", 	"Spenedypp, suspensjon", 	"spenedypp, susp");
    Add("743", 	"Depotkapsel, hard", 	"depotkaps");
    Add("744", 	"Depotkapsel, myk", 	"depotkaps");
    Add("745", 	"Depotøyedråper, oppløsning", 	"depotøyedr, oppl");
    Add("746", 	"Pulver til mikstur, oppløsning", 	"pulv til mikst");
    Add("747", 	"Dråper, emulsjon", 	"dråper, emul");
    Add("748", 	"Dråper, oppløsning", 	"dråper, oppl");
    Add("749", 	"Dråper, suspensjon", 	"dråper, susp");
    Add("750", 	"Dråper, væske", 	"dråper");
    Add("751", 	"Emulsjon og lyofilisat til injeksjonsvæske, suspensjon", 	"emul+lyofil til inj");
    Add("752", 	"Rektalvæske, emulsjon", 	"rektalvæske, emul");
    Add("753", 	"Rektalvæske, oppløsning", 	"rektalvæske, oppl");
    Add("754", 	"Rektalvæske, suspensjon", 	"rektalvæske, susp");
    Add("755", 	"Rekonstitueringsvæske til øyebadevann", 	"rekonst væske til øyebad");
    Add("756", 	"Rekonstitueringsvæske til øyedråper", 	"rekonst væske til øyedr");
    Add("757", 	"Påflekkingsvæske, emulsjon", 	"påflekkingsvæske, emul");
    Add("758", 	"Påflekkingsvæske, oppløsning", 	"påflekkingsvæske, oppl");
    Add("759", 	"Påflekkingsvæske, suspensjon", 	"påflekkingsvæske, susp");
    Add("760", 	"Påhellingsvæske, emulsjon", 	"påhellingsvæske, emul");
    Add("761", 	"Påhellingsvæske, oppløsning", 	"påhellingsvæske, oppl");
    Add("762", 	"Påhellingsvæske, suspensjon", 	"påhellingsvæske, susp");
    Add("763", 	"Pulver til vaginalvæske, oppløsning", 	"pulv til vag væske, oppl");
    Add("764", 	"Pulver til urtete", 	"pulv til urtete");
    Add("765", 	"Endotrakeopulmonal instillasjonsvæske, oppløsning", 	"endotrakeopulm inst væske, oppl");
    Add("766", 	"Endotrakeopulmonal instillasjonsvæske, suspensjon", 	"endotrakeopulm inst væske, susp");
    Add("767", 	"Pulver til rektalvæske, oppløsning", 	"pulv til rektalvæske, oppl");
    Add("768", 	"Pulver til rektalvæske, suspensjon", 	"pulv til rektalvæske, susp");
    Add("769", 	"Enterokapsel, hard", 	"enterokaps");
    Add("770", 	"Enterokapsel, myk", 	"enterokaps");
    Add("771", 	"Pulver til endotrakeopulmonal instillasjonsvæske, oppløsning", 	"pulv til endotrakeopulm inst væske oppl");
    Add("772", 	"Pulver til endotrakeopulmonal instillasjonsvæske, suspensjon", 	"pulv til endotrakeopulm inst væske susp");
    Add("773", 	"Pulver til gurglevann, oppløsning", 	"pulv til gurglevann oppl");
    Add("774", 	"Pulver til konsentrat og væske til infusjonsvæske, oppløsning", 	"pulv til kons+væske til inf, oppl");
    Add("775", 	"Pulver til intravesikaloppløsning/injeksjonsvæske, oppløsning", 	"pulv til intravesikal /inj væske, oppl");
    Add("776", 	"Pulver til intravesikaloppløsning/injeksjons-/infusjonsvæske, oppløsning", 	"pulv til intravesikal /inj /inf væske, oppl");
    Add("777", 	"Gastroenteralvæske, emulsjon", 	"gastroent væske, emul");
    Add("778", 	"Gastroenteralvæske, oppløsning", 	"gastroent væske, oppl");
    Add("779", 	"Gastroenteralvæske, suspensjon", 	"gastroent væske, susp");
    Add("780", 	"Granulat og væske til injeksjonsvæske, suspensjon", 	"granulat+væske til inj, susp");
    Add("781", 	"Granulat og væske til mikstur, suspensjon", 	"granulat+væske til mikst, susp");
    Add("782", 	"Granulat til dråper, oppløsning", 	"granulat til dråper, oppl");
    Add("783", 	"Pulver til konsentrat til infusjonsvæske, dispersjon", 	"pulv til kons til inf væske, disp");
    Add("784", 	"Pulver til konsentrat til infusjonsvæske, oppløsning", 	"pulv til kons til infusjonsvæske, oppl");
    Add("785", 	"Pulver til konsentrat til injeksjons-/infusjonsvæske, oppløsning", 	"pulver til konsentrat til injeksjons-/infusjonsvæske, oppløsning");
    Add("786", 	"Granulat til mikstur, oppløsning", 	"granulat til mikst, oppl");
    Add("787", 	"Granulat til mikstur, suspensjon", 	"granulat til mikst, susp");
    Add("788", 	"Brusepulver og pulver til mikstur, suspensjon", 	"brusepulv+pulv til mikst");
    Add("789", 	"Pulver til injeksjonsvæske, oppløsning", 	"pulv til inj væske, oppl");
    Add("790", 	"Pulver til injeksjonsvæske, suspensjon", 	"pulv til inj væske, susp");
    Add("792", 	"Pulver til injeksjons-/infusjonsvæske, oppløsning", 	"pulv til inj /inf væske, oppl");
    Add("793", 	"Pulver til injeksjonsvæske, suspensjon eller oppløsning", 	"pulv til inj væske, susp eller oppl");
    Add("794", 	"Pulver til infusjonsvæske, dispersjon", 	"pulv til inf væske disp");
    Add("795", 	"Pulver til infusjonsvæske, oppløsning", 	"pulv til inf væske oppl");
    Add("796", 	"Pulver til infusjonsvæske, suspensjon", 	"pulv til inf væske susp");
    Add("797", 	"Pulver til inhalasjonsdamp", 	"pulv til inh damp");
    Add("798", 	"Pulver til inhalasjonsvæske til nebulisator, oppløsning", 	"pulv til inh væske oppl");
    Add("799", 	"Pulver til inhalasjonsvæske til nebulisator, suspensjon", 	"pulv til inh væske oppl");
    Add("800", 	"Pulver og væske til øyedråper, suspensjon", 	"pulv+væske til øyedr susp");
    Add("801", 	"Pulver og væske til øyedråper, oppløsning", 	"pulv+væske til øyedr oppl");
    Add("802", 	"Pulver og væske til øredråper, suspensjon", 	"pulv+væske til øredr susp");
    Add("803", 	"Pulver og væske til sirup", 	"pulv+væske til sirup");
    Add("804", 	"Pulver og væske til pasta til implantasjon", 	"pulv+væske til pasta til impl");
    Add("805", 	"Pulver og væske til oppløsning til bihuler", 	"pulv+væske til bihuleoppl");
    Add("806", 	"Pulver og væske til nesedråper, oppløsning", 	"pulv+væske til nesedr oppl");
    Add("807", 	"Nesedråper, oppløsning", 	"nesedr oppl");
    Add("808", 	"Nesedråper, suspensjon", 	"nesedr susp");
    Add("809", 	"Nesedråper, emulsjon", 	"nesedr emul");
    Add("810", 	"Pulver og væske til infusjonsvæske, oppløsning", 	"pulv+væske til inf oppl");
    Add("811", 	"Pulver og væske til injeksjons-/infusjonsvæske, oppløsning", 	"pulv+væske til inj/inf oppl");
    Add("812", 	"Pulver og væske til injeksjonsvæske, dispersjon", 	"pulv+væske til inj disp");
    Add("813", 	"Pulver og væske til injeksjonsvæske, emulsjon", 	"pulv+væske til inj emul");
    Add("814", 	"Pulver og væske til injeksjonsvæske, oppløsning", 	"pulv+væske til inj oppl");
    Add("815", 	"Pulver og væske til injeksjonsvæske, suspensjon", 	"pulv+væske til inj susp");
    Add("816", 	"Injeksjonsvæske, oppløsning", 	"inj, oppl");
    Add("817", 	"Injeksjonsvæske, emulsjon", 	"inj, emul");
    Add("818", 	"Pulver og væske til mikstur, oppløsning", 	"pulv+væske til mikst oppl");
    Add("819", 	"Pulver og væske til depotinjeksjonsvæske, suspensjon", 	"pulv til depotinj væske, susp");
    Add("820", 	"Pulver og væske til dentalgel", 	"pulv+væske til dentalgel");
    Add("821", 	"Pulver og suspensjon til injeksjonsvæske, suspensjon", 	"pulv+susp til inj susp");
    Add("822", 	"Pulver og væske til endocervikalgel", 	"pulv+væske til endocervikalgel");
    Add("823", 	"Pulver og væske til endotrakeopulmonal instillasjonsvæske, oppløsning", 	"pu+væ til endotrak inst væske oppl");
    Add("824", 	"Pulver og væske til endotrakeopulmonal instillasjonsvæske, suspensjon", 	"pu+væ til endotrak inst væske susp");
    Add("826", 	"Pulver og oppløsning til bikubedispersjon", 	"pulv+oppl til bikubedisp");
    Add("827", 	"Pulver og oppløsning til bikubeoppløsning", 	"pulv+oppl til bikubeoppl");
    Add("828", 	"Pulver og oppløsning til injeksjonsvæske, oppløsning", 	"pulv+oppl til inj oppl");
    Add("829", 	"Oppløsning til inhalasjonsdamp", 	"oppl til inh damp");
    Add("830", 	"Kapsel til inhalasjonsdamp", 	"kaps til inh damp");
    Add("831", 	"Nesespray, emulsjon", 	"nesespray emul");
    Add("832", 	"Nesespray, oppløsning", 	"nesespray oppl");
    Add("833", 	"Nesespray, suspensjon", 	"nesespray susp");
    Add("834", 	"Munnspray, emulsjon", 	"munnspray emul");
    Add("835", 	"Munnspray, oppløsning", 	"munnspray oppl");
    Add("836", 	"Munnspray, suspensjon", 	"munnspray susp");
    Add("837", 	"Munnvann, oppløsning", 	"munnvann oppl");
    Add("838", 	"Munnvann, suspensjon", 	"munnvann susp");
    Add("839", 	"Mikstur/rektalvæske, suspensjon", 	"mikst/rektalvæske susp");
    Add("840", 	"Mikstur, suspensjon", 	"mikst susp");
    Add("841", 	"Mikstur, emulsjon", 	"mikst emul");
    Add("842", 	"Mikstur, oppløsning", 	"mikst oppl");
    Add("843", 	"Medisinsk gass, komprimert", 	"medisinsk gass, komprimert");
    Add("844", 	"Medisinsk gass, flytende", 	"medisinsk gass, flytende");
    Add("845", 	"Hudspray, emulsjon", 	"hudspray, emul");
    Add("846", 	"Hudspray, oppløsning", 	"hudspray, oppl");
    Add("847", 	"Hudspray, pudder", 	"hudspray,pudder");
    Add("848", 	"Hudspray, salve", 	"hudspray, salve");
    Add("849", 	"Hudspray, suspensjon", 	"hudspray, susp");
    Add("850", 	"Lyofilisat til infusjonsvæske, oppløsning", 	"lyofil til inf oppl");
    Add("852", 	"Lyofilisat til injeksjonsvæske, oppløsning", 	"lyofil til inj oppl");
    Add("853", 	"Lyofilisat til injeksjonsvæske, suspensjon", 	"lyofil til inj susp");
    Add("855", 	"Infusjonsvæske, dispersjon", 	"inf, disp");
    Add("856", 	"Infusjonsvæske, emulsjon", 	"inf, emul");
    Add("857", 	"Lyofilisat til okulonasalsuspensjon", 	"lyofil til okulonasalsusp");
    Add("858", 	"Infusjonsvæske, oppløsning", 	"inf, oppl");
    Add("859", 	"Inhalasjonsaerosol, emulsjon", 	"inh aerosol, emul");
    Add("860", 	"Inhalasjonsaerosol, oppløsning", 	"inh aerosol, oppl");
    Add("861", 	"Inhalasjonsaerosol, suspensjon", 	"inh aerosol, susp");
    Add("862", 	"Lyofilisat og væske til okulonasalsuspensjon", 	"lyofil+væske til okulonasalsusp");
    Add("863", 	"Inhalasjonspulver, dosedispensert" ,	"inh pulv");
    Add("864", 	"Inhalasjonsvæske til nebulisator, dispersjon", 	"inh væske, disp");
    Add("865", 	"Lyofilisat og væske til injeksjonsvæske, suspensjon", 	"lyofil+væske til inj susp");
    Add("866", 	"Lyofilisat og væske til injeksjonsvæske, oppløsning", 	"lyofil+væske til inj oppl");
    Add("867", 	"Lyofilisat og væske til injeksjonsvæske, emulsjon", 	"lyofil+væske til inj emul");
    Add("868", 	"Inhalasjonsvæske til nebulisator, emulsjon", 	"inh væske, emul");
    Add("869", 	"Inhalasjonsvæske til nebulisator, oppløsning", 	"inh væske, oppl");
    Add("870", 	"Inhalasjonsvæske til nebulisator, suspensjon", 	"inh væske, susp");
    Add("871", 	"Inhalasjonsvæske, oppløsning", 	"inh væske, oppl");
    Add("872", 	"Konsentrat til mikstur, suspensjon", 	"kons til mikst susp");
    Add("873", 	"Konsentrat til mikstur, oppløsning", 	"kons til mikst oppl");
    Add("874", 	"Injeksjons-/infusjonsvæske, emulsjon", 	"inj/inf, emul");
    Add("875", 	"Konsentrat til liniment, oppløsning", 	"kons til lin oppl");
    Add("876", 	"Konsentrat til injeksjonsvæske, oppløsning", 	"kons til inj oppl");
    Add("877", 	"Konsentrat til infusjonsvæske, oppløsning", 	"kons til inf oppl");
    Add("878", 	"Konsentrat til infusjonsvæske, emulsjon", 	"kons til inf emul");
    Add("879", 	"Konsentrat til infusjonsvæske, dispersjon", 	"kons til inf disp");
    Add("880", 	"Injeksjons-/infusjonsvæske, oppløsning", 	"inj/inf, oppl");
    Add("882", 	"Konsentrat til bad, suspensjon", 	"kons til bad susp");
    Add("883", 	"Konsentrat til bad, oppløsning", 	"kons til bad oppl");
    Add("884", 	"Konsentrat til bad, emulsjon", 	"kons til bad emul");
    Add("885", 	"Konsentrat og væske til liniment, oppløsning", 	"kons+væske til lin oppl");
    Add("886", 	"Konsentrat og væske til konsentrat til oralspray, suspensjon", 	"kons+væske til kons til oralspray susp");
    Add("887", 	"Konsentrat og væske til injeksjonsvæske, oppløsning", 	"kons+væske til inj oppl");
    Add("888", 	"Konsentrat og væske til injeksjonsvæske, suspensjon", 	"kons til inj væske, susp");
    Add("889", 	"Injeksjonsvæske, dispersjon", 	"inj, disp");
    Add("890", 	"Kapsel, myk", 	"kaps");
    Add("891", 	"Kapsel, hard", 	"kaps");
    Add("892", 	"Kapsel med modifisert frisetting, myk", 	"kaps modif frisett");
    Add("893", 	"Kapsel med modifisert frisetting, hard", 	"kaps modif frisett");
    Add("894", 	"Intravesikaloppløsning/injeksjonsvæske, oppløsning", 	"intravesikaloppl/inj");
    Add("895", 	"Intrauterinvæske, suspensjon", 	"intrauterinvæske susp");
    Add("896", 	"Intrauterinvæske, oppløsning", 	"intrauterinvæske, oppløsning");
    Add("897", 	"Intrauterinvæske, emulsjon", 	"intrauterinvæske emul");
    Add("898", 	"Injeksjonsvæske/prikktest, oppløsning", 	"inj/prikktest");
    Add("899", 	"Intraruminalinnlegg, kontinuerlig frisetting", 	"intraruminalinnl kont frisett");
    Add("900", 	"Enteropulver til mikstur, suspensjon", 	"enteropulv til mikst, susp");
    Add("904", 	"Impregnert vattpinne til bruk på hud", 	"impreg vattpinne til bruk på hud");
    Add("905", 	"Enteropulver og væske til mikstur, suspensjon", 	"enteropulv og væske til mikst, susp");
    Add("906", 	"Brusepulver og suspensjon til mikstur, suspensjon", 	"brusepulv+susp til mikst");
    Add("907", 	"Sublingvalt lyofilisat", 	"Sublingvalt lyofil");
    Add("908", 	"Konsentrat til liniment, emulsjon", 	"kons til lin emul");
    Add("909", 	"Granulat, filmdrasjert", 	"gran filmdrasjert");
    Add("910", 	"Enteromikstur, suspensjon", 	"enteromikst, susp");
    Add("911", 	"Urethralsalve", 	"urethralsalve");
    Add("912", 	"Depotinjeksjonsvæske, oppløsning", 	"depotinj væske, oppl");
    Add("913", 	"Depotinjeksjonsvæske, suspensjon", 	"depotinj væske, susp");
    Add("914", 	"Granulat til bruk i drikkevann/melk", 	"granulat til drikkevann/melk");
    Add("915", 	"Inhalasjonsdamp", 	"inh damp");
    Add("916", 	"Depotmikstur, suspensjon", 	"depotmikst, susp");
    Add("917", 	"Mikstur med modifisert frisetting, suspensjon", 	"mikst modif frisetting, susp");
    Add("918", 	"Øre-/øyedråper, oppløsning", 	"øre/øyedr,oppl");
}

constexpr VolvenRecallCode::VolvenRecallCode() {
    Add("1", "Fornying");
    Add("2", "Seponering");
    Add("3", "Fornying med endring");
    Add("4", "Annen");
    Add("5", "Administrativ sletting");
}

constexpr VolvenCessationCode::VolvenCessationCode() {
    Add("A", "Avsluttet behandling");
    Add("D", "Dobbeltoppføring");
    Add("I", "Interaksjon med annet legemiddel");
    Add("L", "Legemiddelreaksjon");
    Add("M", "Manglende effekt");
    Add("P", "Pasienten har ikke brukt legemiddelet");
    Add("S", "Pasienten har selv sluttet med legemiddelet");
    Add("X", "Annen årsak");
}

constexpr CaveSourceOfInformation::CaveSourceOfInformation() {
    Add("1", "Resultat av tester/analyser");
    Add("2", "Observert av behandlende lege");
    Add("3", "Pasientens egne opplysninger");
    Add("4", "Pårørendes opplysninger");
    Add("5", "Hentet fra tidligere journal");
    Add("6", "Annet");
    Add("7", "Opplyst av ansvarlig behandler");
}

constexpr CaveTypeOfReaction::CaveTypeOfReaction() {
    Add("1", "Anafylaktisk reaksjon");
    Add("2", "Blodtrykksfall");
    Add("3", "Alvorlig arytmi");
    Add("4", "Obstruksjon i øvre luftveier (inkl larynxødem)");
    Add("5", "Obstruksjon i nedre luftveier (inkl astma)");
    Add("6", "Uspesifisert tung pust");
    Add("7", "Redusert bevissthet/forvirring");
    Add("8", "Generaliserte kramper");
    Add("9", "Angioødem/generalisert urticaria");
    Add("10", "Hudreaksjon INA");
    Add("12", "Irritasjon i slimhinner");
    Add("13", "Oppkast, diaré, magesmerter");
    Add("14", "Nyresvikt/redusert nyrefunksjon");
    Add("15", "Blodaplasier/bloddysplasier");
    Add("16", "Leversvikt/redusert leverfunksjon");
    Add("17", "Rhabdomyolyse");
    Add("18", "Annen alvorlig reaksjon");
    Add("20", "Ukjent reaksjon");
}

constexpr CaveVerificationStatus::CaveVerificationStatus() {
    Add("unconfirmed", 	"Unconfirmed");
    Add("confirmed", 	"Confirmed");
    Add("refuted", 	"Refuted");
    Add("entered-in-error", 	"Entered in Error");
}

static VolvenMedicamentForm GetVolvenMedicamentFormV() {
    return {};
}
static VolvenRecallCode GetVolvenRecallCodeV() {
    return {};
}
static VolvenCessationCode GetVolvenCessationCodeV() {
    return {};
}
static CaveSourceOfInformation GetCaveSourceOfInformationV() {
    return {};
}
static CaveTypeOfReaction GetCaveTypeOfReactionV() {
    return {};
}
static CaveVerificationStatus GetCaveVerificationStatusV() {
    return {};
}

std::vector<MedicalCodedValue> MedicalCodedValue::GetVolvenMedicamentForm() {
    return GetVolvenMedicamentFormV().values;
}

std::vector<MedicalCodedValue> MedicalCodedValue::GetVolvenRecallCode() {
    return GetVolvenRecallCodeV().values;
}

std::vector<MedicalCodedValue> MedicalCodedValue::GetVolvenCessationCode() {
    return GetVolvenCessationCodeV().values;
}

std::vector<MedicalCodedValue> MedicalCodedValue::GetCaveSourceOfInformation() {
    return GetCaveSourceOfInformationV().values;
}

std::vector<MedicalCodedValue> MedicalCodedValue::GetCaveTypeOfReaction() {
    return GetCaveTypeOfReactionV().values;
}

std::vector<MedicalCodedValue> MedicalCodedValue::GetCaveVerificationStatus() {
    return GetCaveVerificationStatusV().values;
}
