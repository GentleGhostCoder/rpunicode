// Rewritten version using ICU -> special chars not working with id2, e.g. "❌.gaskam.com" - ""xn--1di.gaskam.com"
#include <Rcpp.h>
#include <unicode/uidna.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
#include <string>

// Helper function to convert UnicodeString to std::string
std::string UnicodeStringToStdString(const icu::UnicodeString& ustr) {
  std::string s;
  ustr.toUTF8String(s);
  return s;
}

// Function to check if the domain is encoded in Punycode
bool isPunycode(const std::string& domain) {
  return domain.find("xn--") != std::string::npos;
}

// Function to encode Unicode domains to Punycode
//' @param domains character vector of Unicode domains
//' @name puny_encode
//' @title Encode Unicode domains to Punycode
//' @return a character vector of Punycode domains
//' @export
// [[Rcpp::export]]
Rcpp::CharacterVector puny_encode(Rcpp::CharacterVector domains) {
 UErrorCode status = U_ZERO_ERROR;
 UIDNAInfo info = UIDNA_INFO_INITIALIZER;
 UIDNA* idna = uidna_openUTS46(UIDNA_NONTRANSITIONAL_TO_ASCII, &status);

 int32_t input_size = domains.size();
 Rcpp::CharacterVector output(input_size);

 for (int32_t i = 0; i < input_size; i++) {
   std::string input_domain = Rcpp::as<std::string>(domains[i]);
   icu::UnicodeString unicode_domain = icu::UnicodeString::fromUTF8(input_domain);

   // Prepare output buffer
   UChar dest[256]; // Adjust size as needed
   int32_t destCapacity = sizeof(dest) / sizeof(dest[0]);
   int32_t resultLength = uidna_nameToASCII(idna, unicode_domain.getBuffer(), unicode_domain.length(), dest, destCapacity, &info, &status);

   if (U_SUCCESS(status)) {
     icu::UnicodeString result(dest, resultLength);
     output[i] = UnicodeStringToStdString(result);
   } else {
     output[i] = input_domain; // Fallback to input on error
   }
   status = U_ZERO_ERROR; // Reset status for next iteration
 }

 uidna_close(idna);
 return output;
}

// Function to decode Punycode domains to Unicode with normalization
//' @param domains character vector of Punycode domains
//' @name puny_decode
//' @title Decode Punycode domains to Unicode with normalization
//' @return a character vector of Unicode domains
//' @export
// [[Rcpp::export]]
Rcpp::CharacterVector puny_decode(Rcpp::CharacterVector domains) {
 UErrorCode status = U_ZERO_ERROR;
 UIDNAInfo info = UIDNA_INFO_INITIALIZER;
 UIDNA* idna = uidna_openUTS46(UIDNA_NONTRANSITIONAL_TO_ASCII, &status);

 // Get the normalizer for NFC
 const UNormalizer2* normalizer = unorm2_getInstance(NULL, "nfc", UNORM2_COMPOSE, &status);

 int32_t input_size = domains.size();
 Rcpp::CharacterVector output(input_size);

 for (int32_t i = 0; i < input_size; i++) {
   std::string input_domain = Rcpp::as<std::string>(domains[i]);

   // Skip decoding if not Punycode
   if (!isPunycode(input_domain)) {
     output[i] = input_domain;
     continue;
   }

   icu::UnicodeString unicode_domain = icu::UnicodeString::fromUTF8(input_domain);

   // Allocate a buffer for the normalized output
   UChar normalized_buffer[256]; // Adjust size as needed
   int32_t normalized_length = unorm2_normalize(normalizer, unicode_domain.getBuffer(), unicode_domain.length(), normalized_buffer, sizeof(normalized_buffer)/sizeof(normalized_buffer[0]), &status);

   if (U_SUCCESS(status)) {
     // Create a UnicodeString from the normalized buffer
     icu::UnicodeString normalized_domain(normalized_buffer, normalized_length);

     // Now proceed with uidna_nameToUnicode as before
     UChar dest[256]; // Adjust size as needed
     int32_t destCapacity = sizeof(dest) / sizeof(dest[0]);
     int32_t resultLength = uidna_nameToUnicode(idna, normalized_domain.getBuffer(), normalized_domain.length(), dest, destCapacity, &info, &status);

     if (U_SUCCESS(status)) {
       icu::UnicodeString result(dest, resultLength);
       output[i] = UnicodeStringToStdString(result);
     } else {
       output[i] = input_domain; // Fallback to input on error
     }
   } else {
     output[i] = input_domain; // Fallback to input on normalization error
   }
   status = U_ZERO_ERROR; // Reset status for next iteration
 }

 uidna_close(idna);
 return output;
}
