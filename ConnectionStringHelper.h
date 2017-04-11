#pragma once

#include <Arduino.h>

class ConnectionStringHelper
{
private:
  typedef struct type_knv
  {
	String keyword;
	String value;
  } KeywordAndValue;
	
  KeywordAndValue *_knv;
  int _tokenCount;
  
  const static String CODES;

  int findTokens(const String &connectionString);
  int countTokens(const String &connectionString);
  
public:
  ConnectionStringHelper(const String &connectionString);
  ~ConnectionStringHelper();
  String getKeywordValue(const String &keyword);
  void urlEncode(const String url, String &result);
  void encodeBase64(const uint8_t *input, int inputLength, String &result);
  int decodeBase64(const String input, uint8_t *output, int outputLen);
  void hashIt(const String data, uint8_t *key, size_t keyLength, String &result);
  void generatePassword(const String hostName, const String deviceId, const String sak, int32_t tokenTime, String &result);
};
