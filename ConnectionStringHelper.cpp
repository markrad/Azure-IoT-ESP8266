#include <Arduino.h>

#include "sha256.h"

#include "ConnectionStringHelper.h"


const String ConnectionStringHelper::CODES = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/*
 * Constructor
 * 
 *  connectionString          Azure IoT hub device connection string
 */
ConnectionStringHelper::ConnectionStringHelper(const String &connectionString)
{
  _tokenCount = countTokens(connectionString);

  _knv = _tokenCount == 0? NULL : new KeywordAndValue[_tokenCount];

  if (_knv && findTokens(connectionString) == -1)
  {
    delete [] _knv;
    _knv = NULL;
  }
}

/*
 * Destructor
 */
ConnectionStringHelper::~ConnectionStringHelper()
{
  if (_knv)
	delete [] _knv;

  _knv = NULL;
}

/*
 * getKeywordValue: Returns the value from the connection string for
 * the specified keyword.
 * 
 *  keyword:          Self explanatory
 *  
 *  Returns the keyword value or and empty string if not found
 */
String ConnectionStringHelper::getKeywordValue(const String &keyword)
{
  if (_knv == NULL)
    return "";
    
  String kvWork = keyword;
  
  kvWork.toLowerCase();

  for (int i = 0; i < _tokenCount; i++)
  {
    if (kvWork == _knv[i].keyword)
      return _knv[i].value;
  }

  return "";
}
//
// Encode string for URL
void ConnectionStringHelper::urlEncode(const String url, String &result)
{
  static const char *hex = "0123456789ABCDEF";
  static const String specials = "-._";
  
  result = "";

  for (int i = 0; i < url.length(); i++)
  {
    if (('a' <= url[i] && url[i] <= 'z') ||
        ('A' <= url[i] && url[i] <= 'Z') ||
        ('0' <= url[i] && url[i] <= '9') ||
        (-1 != specials.indexOf(url[i])))
    {
      result += url[i];
    }
    else
    {
      result += '%';
      result += hex[url[i] >> 4];
      result += hex[url[i] & 15];
    }
  }
}

//
// Encodes the input into Base64
void ConnectionStringHelper::encodeBase64(const uint8_t *input, int inputLength, String &result)
{
  result = "";

  int8_t b;

  for (int i = 0; i < inputLength; i += 3)
  {
    b = (input[i] & 0xfc) >> 2;
    result += CODES[b];
    b = (input[i] & 0x03) << 4;
    
    if (i + 1 < inputLength)      
    {
      b |= (input[i + 1] & 0xF0) >> 4;
      result += CODES[b];
      b = (input[i + 1] & 0x0F) << 2;
      
      if (i + 2 < inputLength)  
      {
        b |= (input[i + 2] & 0xC0) >> 6;
        result += CODES[b];
        b = input[i + 2] & 0x3F;
        result += CODES[b];
      } 
      else  
      {
        result += CODES[b];
        result += ('=');
      }
    } 
    else      
    {
      result += CODES[b];
      result += ("==");
    }    
  }
}

//
// Decodes from Base64
int ConnectionStringHelper::decodeBase64(const String input, uint8_t *output, int outputLen)
{
  int b[4];

  if (input.length() % 4 != 0)
    return -1;    // Base64 string's length must be a multiple of 4
    
  int requiredLen = (input.length() * 3) / 4 - (input.indexOf('=') > 0 ? (input.length() - input.indexOf('=')) : 0);

  if (outputLen == 0 || output == NULL)
    return requiredLen;

  if (requiredLen > outputLen)
    return -2;    // Output buffer is too short

  int j = 0;

  for (int i = 0; i < input.length(); i += 4)
  {
    b[0] = CODES.indexOf(input[i]);
    b[1] = CODES.indexOf(input[i + 1]);
    b[2] = CODES.indexOf(input[i + 2]);
    b[3] = CODES.indexOf(input[i + 3]);

    output[j++] = (byte) ((b[0] << 2) | (b[1] >> 4));

    if (b[2] < 64)
    {
      output[j++] = (byte) ((b[1] << 4) | (b[2] >> 2));
      
      if (b[3] < 64)  
      {
        output[j++] = (byte) ((b[2] << 6) | b[3]);
      }
    }
  }

  return requiredLen;
}

//
// Returns the hashed value of the data and the key
void ConnectionStringHelper::hashIt(const String data, uint8_t *key, size_t keyLength, String &result)
{
  uint8_t signedOut[32];
  String work;

  generateHash(signedOut, (uint8_t *)data.c_str(), (size_t) data.length(), key, keyLength);
  encodeBase64(signedOut, sizeof(signedOut), work);
  urlEncode(work, result);
}

//
// Generate the SAS token for the IoT Hub
void ConnectionStringHelper::generatePassword(const String hostName, const String deviceId, const String sak, int32_t tokenTime, String &result)
{
  String uri;
  
  urlEncode(hostName + "/devices/" + deviceId, uri);
  String toSign = uri + "\n" + String(tokenTime);
  uint8_t *key;
  int keyLen;

  keyLen = decodeBase64(sak, NULL, 0);
  key = new byte[keyLen];
  keyLen = decodeBase64(sak, key, keyLen);
  
  String password;
  hashIt(toSign, key, keyLen, password);

  delete [] key;
  
  result = "SharedAccessSignature sr=" + uri + "&sig=" + password + "&se=" + String(tokenTime);
}

/*
 * Private method
 */
int ConnectionStringHelper::countTokens(const String &connectionString)
{
  if (connectionString.length() == 0)
    return 0;

  if (connectionString.indexOf('=') < 0)
    return 0;

  if (connectionString.startsWith("="))
    return 0;

  int result = connectionString.endsWith(";")? 0 : 1;
  int searchBegin = 0;

  while (-1 != (searchBegin = connectionString.indexOf(";", searchBegin)))
  {
    result++;
    searchBegin++;
  }

  return result;
}

/*
 * Private method
 */
int ConnectionStringHelper::findTokens(const String &connectionString)
{
  String connectionStringWork = connectionString + ";";
  int tokenNumber = 0;
  int searchBegin = 0;
  int keywordBegin = 0;
  int keywordEnd;
  
  while (-1 != (searchBegin = connectionStringWork.indexOf(";", searchBegin)))
  {
    keywordEnd = connectionStringWork.indexOf("=", keywordBegin);

    if (keywordEnd > searchBegin)
    {
      return -1;
    }

    _knv[tokenNumber].keyword = connectionStringWork.substring(keywordBegin, keywordEnd);
    _knv[tokenNumber].value = connectionStringWork.substring(keywordEnd + 1, searchBegin);
    _knv[tokenNumber].keyword.toLowerCase();
    keywordBegin = ++searchBegin;
    tokenNumber++;
  }

  return --tokenNumber;
}

