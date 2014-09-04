
#ifndef THORSANVIL_PARSER_PARSER_PARSER_INTERFACE_H
#define THORSANVIL_PARSER_PARSER_PARSER_INTERFACE_H

#include "ParserDom.h"
#include "ParserDomVisit.h"
#include "ParserException.h"
#include "LexerParser.h"

#include <string>
#include <iostream>
#include <sstream>


namespace ThorsAnvil
{
    namespace Parser
    {

    class LexerParser;
    struct ParserInterface;

/*
 * Exceptions:
 *      class InvalidConversion
 *      class ParsingError
 *
 * Interface for Parsing
 *      struct ParserInterface
 *      struct ParserCleanInterface
 *      struct ParserLogInterface   (template that sits in-front of another interface to log calls.)
 *      struct ParserDomInterface
 *
 */

enum ParserObjectType { ParserMapObject, ParserArrayObject, ParserValueObject, NotSet };

/*
 * The interface injected into the parser that will do the work.
 */
struct KeyGenVisitor: public ParserValueConstVisitor
{
    virtual void visit(ParserStringItem const&)        {}
    virtual void visit(ParserNumberItem const&)        {}
    virtual void visit(ParserBoolItem const&)          {}
    virtual void visit(ParserNULLItem const&)          {}
    virtual void visit(ParserMapItem const&)           {}
    virtual void visit(ParserArrayItem const&)         {}
    virtual void visit(ParserMap const&, Storage const&, Storage const&) {}
    virtual void visit(ParserArray const&)             {}
    virtual std::string  getKey() { return "";}
};
struct ParserInterface
{
    KeyGenVisitor&       keyGenVisitor;
    ParserInterface(KeyGenVisitor& keyGenVisitor)
        : keyGenVisitor(keyGenVisitor)
    {}
    virtual ~ParserInterface()  {}
    std::string mapValueToKey(ParserValue& key)
    {
       //key.accept(keyGenVisitor);
       //return keyGenVisitor.getKey();
        return key.keyValue();
    }
    virtual void            done(ParserObjectType type, ParserValue* result)    = 0;
    virtual void            mapOpen()                                           = 0;
    virtual void            mapClose()                                          = 0;
    virtual ParserMap*      mapCreate()                                         = 0;
    virtual ParserMap*      mapCreate(ParserMapValue* val)                      = 0;
    virtual ParserMap*      mapAppend(ParserMap* map, ParserMapValue* val)      = 0;
    virtual ParserMapValue* mapCreateElement(ParserValue* k,ParserValue* val)   = 0;
    virtual ParserValue*    mapKeyNote(ParserValue* k)                          = 0;
    virtual void            arrayOpen()                                         = 0;
    virtual void            arrayClose()                                        = 0;
    virtual ParserArray*    arrayCreate()                                       = 0;
    virtual ParserArray*    arrayCreate(ParserValue* val)                       = 0;
    virtual ParserArray*    arrayAppend(ParserArray* arr, ParserValue* val)     = 0;
    virtual ParserArray*    arrayNote(ParserArray* arr)                         = 0;
    virtual ParserValue*    arrayCreateElement(ParserValue* val)                = 0;
    virtual ParserValue*    valueParseMap(ParserMap* map)                       = 0;
    virtual ParserValue*    valueParseArray(ParserArray* arr)                   = 0;
    virtual ParserValue*    valueParseString(std::string* str)                  = 0;
    virtual ParserValue*    valueParseNumber(int base, int off, std::string* n) = 0;
    virtual ParserValue*    valueParseNumber(std::string* num)                  = 0;
    virtual ParserValue*    valueParseBool(bool value)                          = 0;
    virtual ParserValue*    valueParseNULL(bool okKey = false)                  = 0;
};

struct ParserCleanInterface: ParserInterface
{
    ParserCleanInterface(KeyGenVisitor& keyGenVisitor)
        : ParserInterface(keyGenVisitor)
    {}
    virtual void            done(ParserObjectType, ParserValue* result)         { delete result;}
    virtual void            mapOpen()                                           {}
    virtual void            mapClose()                                          {}
    virtual ParserMap*      mapCreate()                                         { return NULL;}
    virtual ParserMap*      mapCreate(ParserMapValue* val)                      { delete val; return NULL;}
    virtual ParserMap*      mapAppend(ParserMap* map, ParserMapValue* val)      { std::unique_ptr<ParserMapValue> aval(val); delete map; return NULL;}
    virtual ParserMapValue* mapCreateElement(ParserValue* k,ParserValue* val)   { std::unique_ptr<ParserValue> aval(val);    delete k;   return NULL;}
    virtual ParserValue*    mapKeyNote(ParserValue* k)                          { delete k; return NULL;}
    virtual void            arrayOpen()                                         {}
    virtual void            arrayClose()                                        {}
    virtual ParserArray*    arrayCreate()                                       { return NULL;}
    virtual ParserArray*    arrayCreate(ParserValue* val)                       { delete val; return NULL;}
    virtual ParserArray*    arrayAppend(ParserArray* arr, ParserValue* val)     { std::unique_ptr<ParserArray> aarr(arr);    delete val; return NULL;}
    virtual ParserArray*    arrayNote(ParserArray* arr)                         { delete arr; return NULL;}
    virtual ParserValue*    arrayCreateElement(ParserValue* val)                { delete val; return NULL;}
    virtual ParserValue*    valueParseMap(ParserMap* map)                       { delete map; return NULL;}
    virtual ParserValue*    valueParseArray(ParserArray* arr)                   { delete arr; return NULL;}
    virtual ParserValue*    valueParseString(std::string* str)                  { delete str; return NULL;}
    virtual ParserValue*    valueParseNumber(int, int, std::string* num)        { delete num; return NULL;}
    virtual ParserValue*    valueParseNumber(std::string* num)                  { delete num; return NULL;}
    virtual ParserValue*    valueParseBool(bool)                                { return NULL;}
    virtual ParserValue*    valueParseNULL(bool)                                { return NULL;}
};

template<typename T = ParserCleanInterface>
struct ParserLogInterface: ParserInterface
{
    T   actualInterface;
    virtual void            done(ParserObjectType type, ParserValue* result)
    {
        if (result)
        {
            switch(type)
            {
                case ParserMapObject:   std::cout << "ParserObject: ParserMap\n";   break;
                case ParserArrayObject: std::cout << "ParserObject: ParserArray\n"; break;
                case ParserValueObject: std::cout << "ParserObject: ParserValue\n"; break;
                default:                std::cout << "ParserObject: NOT SET\n";     break;
            }
        }
        else
        {
            std::cout << "ParserObject: NULL\n";
        }
        actualInterface.done(type, result);
    }
    template<typename ...Args>
    ParserLogInterface(KeyGenVisitor& keyGenVisitor, Args&&... args)
        : ParserInterface(keyGenVisitor)
        , actualInterface(keyGenVisitor, std::forward<Args>(args)...)
    {}
    virtual void            mapOpen()                                           {std::cout << "mapOpen!\n";                                                   actualInterface.mapOpen();}
    virtual void            mapClose()                                          {std::cout << "ParserMap: { ParserMapValueListOpt }\n";                       actualInterface.mapClose();}
    virtual ParserMap*      mapCreate()                                         {std::cout << "ParserMapValueListOpt: EMPTY\n";                               return actualInterface.mapCreate();}
    virtual ParserMap*      mapCreate(ParserMapValue* val)                      {std::cout << "ParserMapValueList: ParserMapValue\n";                         return actualInterface.mapCreate(val);}
    virtual ParserMap*      mapAppend(ParserMap* map, ParserMapValue* val)      {std::cout << "ParserMapValueList: ParserMapValueList , ParserMapValue\n";    return actualInterface.mapAppend(map, val);}
    virtual ParserMapValue* mapCreateElement(ParserValue* v1,ParserValue* v2)   {std::cout << "ParserMapValue: PARSER_STRING : ParserValue\n";                return actualInterface.mapCreateElement(v1, v2);}
    virtual ParserValue*    mapKeyNote(ParserValue* val)                        {                                                                             return actualInterface.mapKeyNote(val);}
    virtual void            arrayOpen()                                         {std::cout << "arrayOpen!\n";                                                 actualInterface.arrayOpen();}
    virtual void            arrayClose()                                        {std::cout << "ParserArray: [ ParserArrayValueListOpt ]\n";                   actualInterface.arrayClose();}
    virtual ParserArray*    arrayCreate()                                       {std::cout << "ParserArrayValueListOpt: EMPTY\n";                             return actualInterface.arrayCreate();}
    virtual ParserArray*    arrayCreate(ParserValue* val)                       {std::cout << "ParserArrayValueList: ParserArrayValue\n";                     return actualInterface.arrayCreate(val);}
    virtual ParserArray*    arrayAppend(ParserArray* v1, ParserValue* v2)       {std::cout << "ParserArrayValueList: ParserArrayListItem ParserArrayValue\n"; return actualInterface.arrayAppend(v1, v2);}
    virtual ParserArray*    arrayNote(ParserArray* val)                         {std::cout << "ParserArrayListItem: ParserArrayValueList ','\n";              return actualInterface.arrayNote(val);}
    virtual ParserValue*    arrayCreateElement(ParserValue* val)                {std::cout << "ParserArrayValue: ParserValue\n";                              return actualInterface.arrayCreateElement(val);}
    virtual ParserValue*    valueParseMap(ParserMap* val)                       {std::cout << "ParserValue: ParserMap\n";                                     return actualInterface.valueParseMap(val);}
    virtual ParserValue*    valueParseArray(ParserArray* val)                   {std::cout << "ParserValue: ParserArray\n";                                   return actualInterface.valueParseArray(val);}
    virtual ParserValue*    valueParseString(std::string* val)                  {std::cout << "ParserValue: ParserString\n";                                  return actualInterface.valueParseString(val);}
    virtual ParserValue*    valueParseNumber(int base, int off, std::string* n) {std::cout << "ParserValue: ParserNumber\n";                                  return actualInterface.valueParseNumber(base, off, n);}
    virtual ParserValue*    valueParseNumber(std::string* num)                  {std::cout << "ParserValue: ParserNumber\n";                                  return actualInterface.valueParseNumber(num);}
    virtual ParserValue*    valueParseBool(bool val)                            {std::cout << "ParserValue: ParserTrue\n";                                    return actualInterface.valueParseBool(val);}
    virtual ParserValue*    valueParseNULL(bool okKey = false)                  {std::cout << "ParserValue: ParserFalse\n";                                   return actualInterface.valueParseNULL(okKey);}
};

struct ParserDomInterface: ParserCleanInterface
{
    virtual ParserMap*      mapCreate()                                         { return new ParserMap();}
    virtual ParserMap*      mapCreate(ParserMapValue* val)                      {
                                                                                  std::unique_ptr<ParserMapValue>   aval(val);
                                                                                  std::unique_ptr<ParserMap>        amap(new ParserMap());
                                                                                  amap->insert(*this, std::move(aval->first), std::move(aval->second));
                                                                                  return amap.release();
                                                                                }
    virtual ParserMap*      mapAppend(ParserMap* map, ParserMapValue* val)      {
                                                                                  std::unique_ptr<ParserMapValue>   aval(val);
                                                                                  std::unique_ptr<ParserMap>        amap(map);
                                                                                  amap->insert(*this, std::move(aval->first), std::move(aval->second));
                                                                                  return amap.release();
                                                                                }
    virtual ParserMapValue* mapCreateElement(ParserValue* k,ParserValue* val)   { std::unique_ptr<ParserValue>  aval(val);std::unique_ptr<ParserValue> ak(k);
                                                                                  std::unique_ptr<ParserMapValue> result(new ParserMapValue);result->first = std::move(ak);result->second = std::move(aval);
                                                                                  return result.release();
                                                                                }
    virtual ParserValue*    mapKeyNote(ParserValue* k)                          { std::unique_ptr<ParserValue> pk(k); return pk.release();}
    virtual ParserArray*    arrayCreate()                                       { return new ParserArray();}
    virtual ParserArray*    arrayCreate(ParserValue* val)                       { std::unique_ptr<ParserValue>  aval(val);std::unique_ptr<ParserArray>   aarr(new ParserArray());aarr->push_back(aval.release()); return aarr.release();}
    virtual ParserArray*    arrayAppend(ParserArray* arr, ParserValue* val)     { std::unique_ptr<ParserValue>  aval(val);std::unique_ptr<ParserArray>   aarr(arr);            aarr->push_back(aval.release()); return aarr.release();}
    virtual ParserArray*    arrayNote(ParserArray* arr)                         { return arr;}
    virtual ParserValue*    arrayCreateElement(ParserValue* val)                { std::unique_ptr<ParserValue>  aval(val); return aval.release();}

    virtual ParserValue*    valueParseMap(ParserMap* map)                       { std::unique_ptr<ParserMap>     amap(map); return new ParserMapItem(amap);}
    virtual ParserValue*    valueParseArray(ParserArray* arr)                   { std::unique_ptr<ParserArray>   aarr(arr); return new ParserArrayItem(aarr);}
    virtual ParserValue*    valueParseString(std::string* str)                  { std::unique_ptr<std::string> astr(str); return new ParserStringItem(astr);}
    virtual ParserValue*    valueParseNumber(int b, int o, std::string* num)    { std::unique_ptr<std::string> anum(num); return new ParserNumberItem(b, o, anum);}
    virtual ParserValue*    valueParseNumber(std::string* num)                  { std::unique_ptr<std::string> anum(num); return new ParserNumberItem(anum);}
    virtual ParserValue*    valueParseBool(bool value)                          { return new ParserBoolItem(value);}
    virtual ParserValue*    valueParseNULL(bool okKey = false)                  { return new ParserNULLItem(okKey);}

    ParserObjectType                type;
    std::unique_ptr<ParserValue>    result;

    ParserDomInterface(KeyGenVisitor& keyGenVisitor)
        : ParserCleanInterface(keyGenVisitor)
        , type(NotSet)
    {}
    virtual void            done(ParserObjectType valueType, ParserValue* value)
    {
        type    = valueType;
        result.reset(value);
    }
};


template<typename C>
struct TransformParserStringIter: std::iterator<std::input_iterator_tag, char, ptrdiff_t, char*,char&>
{
    C&       cont;
    bool     lastWasSlash;
    int      unicodeCount;
    uint32_t unicodeValue;
    TransformParserStringIter(C& c)
        : cont(c)
        , lastWasSlash(false)
        , unicodeCount(0)
    {}
    TransformParserStringIter& operator++()       {return *this;}
    TransformParserStringIter& operator*()        {return *this;}
    void                     operator=(char x)
    {
        if (unicodeCount)
        {
            if (unicodeCount == 6)
            {
                if (x != '\\')  { throw ThorsAnvil::Parser::ParsingError("Surrogate pair(No Slash): \\uD8xx Must be followed by \\uDCxx");}
                --unicodeCount;
            }
            else if (unicodeCount == 5)
            {
                if (x != 'u')   { throw ThorsAnvil::Parser::ParsingError("Surrogate pair(No u): \\uD8xx Must be followed by \\uDCxx");}
                --unicodeCount;
            }
            else
            {
                unicodeValue <<= 4;
                unicodeValue += ('0' <= x && x <= '9') ? (x - '0') : 10 + (('A' <= x && x <= 'F') ? (x - 'A') : (x - 'a'));
                --unicodeCount;
                if (unicodeCount == 0)
                {
                    if (unicodeValue <= 0x7F)
                    {
                        // Encode as single UTF-8 character
                        cont.push_back(unicodeValue);
                    }
                    else if (unicodeValue <= 0x7FF)
                    {
                        // Encode as two UTF-8 characters
                        cont.push_back(0xC0 |((unicodeValue >>  6)));
                        cont.push_back(0x80 |((unicodeValue >>  0) & 0x3F));
                    }
                    else if (unicodeValue <= 0xFFFF)
                    {
                        if ((unicodeValue & 0xFC00) != 0xD800)
                        {
                            // Encode as three UTF-8 characters
                            cont.push_back(0xE0 |((unicodeValue >> 12)));
                            cont.push_back(0x80 |((unicodeValue >>  6) & 0x3F));
                            cont.push_back(0x80 |((unicodeValue >>  0) & 0x3F));
                        }
                        else
                        {
                            // We have a found first part of surrogate pair
                            unicodeCount    = 6;
                        }
                    }
                    else
                    {
                        // Surrogate pair
                        if ((unicodeValue & 0xFC00FC00) != 0xD800DC00){ throw ThorsAnvil::Parser::ParsingError("Surrogate pair(No DC): \\uD8xx Must be followed by \\uDCxx");}

                        // Decode surrogate pair
                        unicodeValue    = 0x00010000 | ((unicodeValue & 0x03FF0000) >> 6) | (unicodeValue & 0x000003FF);

                        // Encode as 4 UTF-8 characters
                        cont.push_back(0xF0 |((unicodeValue >> 18)));
                        cont.push_back(0x80 |((unicodeValue >> 12) & 0x3F));
                        cont.push_back(0x80 |((unicodeValue >>  6) & 0x3F));
                        cont.push_back(0x80 |((unicodeValue >>  0) & 0x3F));
                    }
                }
            }
        }
        else if (lastWasSlash)
        {
            switch(x)
            {
                case '"':   cont.push_back('"');    break;
                case '\\':  cont.push_back('\\');   break;
                case '/':   cont.push_back('/');    break;
                case 'b':   cont.push_back('\b');   break;
                case 'f':   cont.push_back('\f');   break;
                case 'n':   cont.push_back('\n');   break;
                case 'r':   cont.push_back('\r');   break;
                case 't':   cont.push_back('\t');   break;
                case 'u':   unicodeCount = 4; unicodeValue = 0; break;
            }
            lastWasSlash    = false;
        }
        else
        {
            if (x == '\\')
            {   lastWasSlash    = true;
            }
            else
            {   cont.push_back(x);
            }
        }
    }
};
template<typename C> TransformParserStringIter<C> make_TransformParserStringIter(C& cont)   {return TransformParserStringIter<C>(cont);}

    }
}

#endif
