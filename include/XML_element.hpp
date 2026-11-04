//
//  XML_element.hpp
//  XML_element
//
//  Created by Jan Pieter Abrahams on 17/05/2019.
//  Copyright © 2019 Jan Pieter Abrahams. All rights reserved.
//

#ifndef XML_element_h
#define XML_element_h

#include <limits>
#include <algorithm>
#include <vector>
#include <sstream>

// The XML_element class is derived from std::string. It scans an istream for an XML element with  a
// specified tag. The element can contain attibutes and data. The tag itself is stripped and its
// attributes and data can be accessed as a string or parsed further by constructing another XML_element.
//
// After reading the specified Element from istream, the istream is left in the condition that exists
// after having read the Element, but not further. This allows mixing XML straightforwardly with
// binary data. For instance, after having read a XML metadata header, the file is positioned where the
// binary data starts.
//
// Constructor:
//  template <typename Stream>
//  XML_element(std::istream& is, std::string const& tag)
//  XML_element(std::istream&& is, std::string const& tag)
//      istr is skipped until the specified XML tag ('<' + tag + '>') is encountered. Then all the XML data
//      defined by that tag, are parsed into the XML_element, so until '</' + tag + '>' is encountered,
//      and scanning the file stops after having read the end tag. Any attributes of the tag are parsed into
//      XML_element. Tags can be empty (when they end with “/>”), in which case only the attributes of tag
//      are parsed into the XML_element.
//  XML_element(std::istream& is)
//  XML_element(std::istream&& is)
//      istr is skipped until the first occurence of a valid XML tag is encountered. Then this tag is used for
//      creating the XML_element.
//  XML_element(std::string const& xml_string)
//  XML_element(std::string const& xml_string, std::string const& tag)
//      Delegates to the constructor requiring a std::istream, after creating a std::istringstream(xml_string).
//
// Member functions:
//  std::string const attribute(std::string const& att_name) const
//      Returns the attribute of the tag of XML_element that has the name 'att_name'. An XML_element line does
//      not have to have attributes. If none of the attributes is named 'att_name', an empty string is returned.
//  std::string const attribute(std::size_t n) const
//      Returns the n-th attribute of an XML_element. If n is larger than the number of attributes, an empty
//      string is returned.
//  std::string const tag()
//      Returns the tag name of the XML_element that was used for extracting it from the XML data. If no tag name
//      was specified when the XML_element was created, an empty string is returned.
//  std::string const tag(std::string const& tag)
//      Returns the XML data encoded by the first occurrence of requested tag as an XML_element.
//  std::vector<XML_element> const multitag(std::string const& tag)
//      Returns all requested tags as a vector, in the order in which they occur in original XML data.
//  std::vector<T> const value<T>(std::string const& tag)
//      Returns a vector of the requested type, containing the numerical values of the requested tag of the
//      XML_element.
//  std::vector<std::vector<T>> const multivalue<T>(std::string const& tag)
//      Returns a vector of all the requested value<T>s.
//
// An example:
//
//    int main(int argc, const char * argv[]) {
//        std::string xml("<Element1 att0=\"plop\">\n  <Element2 att1=\"1\", att2=\"22\"/>\n</Element1>\n");
//        std::cout << "xml stream:\n" << xml << std::endl <<"Parsing produces:\n";
//        std::cout << "Element 1: " << XML_element(xml, "Element1") << std::endl;
//        std::cout << "Attribute att0 of element 1: " << XML_element(xml, "Element1").attribute("att0") << std::endl;
//        std::cout << "Attribute att1 of element 1 does not exist, so empty string is returned: " << '\"' << XML_element(xml, "Element1").attribute("att1") << '\"' << std::endl;
//        std::cout << "Attribute att1 of element 2: " << XML_element(xml, "Element2").attribute("att1") << std::endl;
//        std::cout << "Attribute att2 of element 2: " << XML_element(xml, "Element2").attribute("att2") << std::endl;
//        std::istringstream xml_stream(xml);
//        XML_element(xml_stream, "Element2");
//        std::string rest;
//        xml_stream >> rest;
//        std::cout << "After reading Element2, the xml_stream is now located ath the end of Element2. \nSubsequent output from theis stream produces:\n"<< rest << std::endl;
//
//        return 0;
//    }
//
// Produces the following output:
//
//    xml stream:
//    <Element1 att0="plop">
//      <!-- comment " > -->
//      <values> 1 2 3 4 </values>
//      <Element2 att1="1", att2="22"/>
//    </Element1>
//
//    Parsing produces:
//    Element 1:
//      <!-- comment " > -->
//      <values> 1 2 3 4 </values>
//      <Element2 att1="1", att2="22"/>
//    Attribute att0 of element 1: plop
//    Attribute att1 of element 1 does not exist, so an empty string is returned: ""
//    Attribute att1 of element 2: 1
//    Attribute att2 of element 2: 22
//    After reading Element2, the xml_stream is now located at the end of Element2.
//    Subsequent output from this stream produces:
//    </Element1>

namespace jpa {

/**
 * @class XML_element
 * @brief Represents an XML element with attributes and data.
 *
 * The XML_element class is derived from std::string and is used to parse and store XML data.
 * It scans an input stream for an XML element with a specified tag, extracts its attributes,
 * and stores the its XML data defined inside the element. The tag itself is stripped, and attributes
 * and data can be accessed as strings. Or data can be parsed if they contain XML_elements.
 *
 * After reading the specified element from the input stream, the stream is left in a state that
 * allows mixing XML with binary data. The stream is positioned at the first location after the closing '/>' tag
 * of the requested XML_element. This can be the location where the binary data starts. For instance,
 * after having read a XML metadata header, the file is positioned where the binary data starts.
 *
 * An example:
 * @code{.cpp}
 *    int main(int argc, const char * argv[]) {
 *        std::string xml("<Element1 att0=\"plop\">\n  <Element2 att1=\"1\", att2=\"22\"/>\n</Element1>\n");
 *        std::cout << "xml stream:\n" << xml << std::endl <<"Parsing produces:\n";
 *        std::cout << "Element 1: " << XML_element(xml, "Element1") << std::endl;
 *        std::cout << "Attribute att0 of element 1: " << XML_element(xml, "Element1").attribute("att0") << std::endl;
 *        std::cout << "Attribute att1 of element 1 does not exist, so an empty string is returned: " << '\"' << XML_element(xml, "Element1").attribute("att1") << '\"' << std::endl;
 *        std::cout << "Attribute att1 of element 2: " << XML_element(xml, "Element2").attribute("att1") << std::endl;
 *        std::cout << "Attribute att2 of element 2: " << XML_element(xml, "Element2").attribute("att2") << std::endl;
 *        std::istringstream xml_stream(xml);
 *        XML_element(xml_stream, "Element2");
 *        std::string rest;
 *        xml_stream >> rest;
 *        std::cout << "After reading Element2, the xml_stream is now located at the end of Element2. \nSubsequent output from this stream produces:\n"<< rest << std::endl;
 *        return 0;
 *    }
 * @endcode
 *
 * Produces the following output:
 * <pre>
 *    xml stream:
 *    <Element1 att0="plop">
 *      <!-- comment " > -->
 *      <values> 1 2 3 4 </values>
 *      <Element2 att1="1", att2="22"/>
 *    </Element1>
 *
 *    Parsing produces:
 *    Element 1:
 *      <!-- comment " > -->
 *      <values> 1 2 3 4 </values>
 *      <Element2 att1="1", att2="22"/>
 *    Attribute att0 of element 1: plop
 *    Attribute att1 of element 1 does not exist, so an empty string is returned: ""
 *    Attribute att1 of element 2: 1
 *    Attribute att2 of element 2: 22
 *    After reading Element2, the xml_stream is now located at the end of Element2.
 *    Subsequent output from this stream produces:
 *    </Element1>
 * </pre>
 */

class XML_element : public std::string {

    class s_param {
        friend XML_element;
        s_param(std::string const& str, std::size_t frm, std::size_t siz) : s(str), from(frm), size(siz) {}
        friend void swap(s_param& left, s_param &right) {std::swap(left.from, right.from); std::swap(left.size, right.size);}
        std::string const& s;
        std::size_t from;
        std::size_t size;
        
        std::size_t to() const noexcept {return from + size;}
        
    public:
        s_param(s_param const& other) : s(other.s), from(other.from), size(other.size) {}
        constexpr auto& operator=(s_param const& other) noexcept {from = other.from; size = other.size; return *this;}
        
        template <typename T>
        operator const T() const {
            T r;
            std::string const s1(s, from, size);
            std::istringstream(s1) >> r;
            return r;
        }
        
        operator const std::string() const { return std::string(s, from, size); }
        
        std::string const tag() const {
            if (s[from] != '<') return "";
            std::size_t tagsize = s.find_first_of(" >", from) - from - 1;
            if (s[from + tagsize] == '/') --tagsize;
            return std::string(s, from + 1, tagsize);
        }
        
        operator XML_element() const { return XML_element(std::string(s, from, size), tag()); }
    };
    
public:

    /**
     * @brief Constructor for XML_element with specified tag.
     *
     * Constructs an XML_element by parsing XML data from the input stream 'istr' with the specified XML tag.
     * When a character sequence '<' + tag + '>' is encountered, it checks if the element is empty (which
     * is the case when the last two characters of the tag are '/>'), in which case it only stores the attributes. If
     * the the tag is not empty, it scans for the end tag: '</' + tag + '>', and stores the data between the
     * beginning and end tag. The input stream is left at the position immediately after the closing '>' character
     * of the specified XML element. If no element is found, the file pointer is 'eof', and no data are stored.
     *
     * @param istr The input stream containing the XML data.
     * @param tag  The tag to search for in the XML data.
     */
    XML_element(std::istream&& istr, std::string const& tag) :
    std::string(),
    d_tag(tag),
    d_attributes(f_get_attributes(istr)) {
        if (istr.good() && !f_prev_is(istr, "/>") && d_tag_found) { // only do if this is not an empty element
            static_cast<std::string&>(*this) = f_read_element(istr, tag);
            d_param = f_find_all_params(std::string(*this));
        }
    }
    
    /**
     * @brief Constructor for XML_element with specified tag.
     *
     * @see XML_element(std::istream&& istr, std::string const& tag)
     * @param istr The input stream containing the XML data.
     * @param tag  The tag to search for in the XML data.
     */
    XML_element(std::istream& istr, std::string const& tag) : XML_element(std::move(istr), tag)  {}
    
    /**
     * @brief Constructor for XML_element with specified tag.
     *
     * Constructs an XML_element by parsing XML data from the input string 'xml_string' with the specified
     * XML tag.
     *
     * @see XML_element(std::istream&& istr, std::string const& tag)
     * @param xml_string The input string containing the XML data.
     * @param tag  The tag to search for in the XML data.
     */
    XML_element(std::string const& xml_string, std::string const& tag) : XML_element(std::istringstream(xml_string), tag)  {}

    
    /**
     * @brief Constructor for XML_element without specifying the tag.
     *
     * Constructs an XML_element by parsing XML data from the input stream 'istr' without specifying a tag.
     * The tag is determined by searching for the first valid XML tag in the input stream.
     *
     * @see XML_element(std::istream&& istr, std::string const& tag)
     * @param istr The input stream containing the XML data.
     */
    XML_element(std::istream&& istr) :
    XML_element(std::move(istr), std::string([&] {
        f_read_upto(istr, '<');
        auto tag_start = istr.tellg() - std::basic_istream<char>::pos_type(1);
        std::string tag = f_read_upto_any_of(istr, "/> ");
        istr.seekg(tag_start);
        return tag;
    }())) {}
    
    /**
     * @brief Constructor for XML_element without specifying the tag.
     *
     * Constructs an XML_element by parsing XML data from the input stream 'istr' without specifying a tag.
     * The tag is determined by searching for the first valid XML tag in the input stream.
     *
     * @see XML_element(std::istream&& istr, std::string const& tag)
     * @param istr The input stream containing the XML data.
     */
    XML_element(std::istream& istr) : XML_element(std::move(istr))  {}
    
    /**
     * @brief Constructor for XML_element without specifying the tag.
     *
     * Constructs an XML_element by parsing XML data from the input string 'istr' without specifying a tag.
     * The tag is determined by searching for the first valid XML tag in the input stream.
     *
     * @see XML_element(std::istream&& istr, std::string const& tag)
     * @param xml_string The input string containing the XML data.
     */
    XML_element(std::string const& xml_string) : XML_element(std::istringstream(xml_string))  {}
    
    /**
     * @brief Retrieves the attribute value by name.
     *
     * Returns the attribute value with the specified 'name' from the XML element's attributes.
     *
     * @param name The name of the attribute to retrieve.
     * @return     The attribute value or an empty string if the attribute is not found.
     */
    std::string const attribute(std::string const& name) const noexcept {
        for (std::size_t i = 0; i <= (d_attributes.size() - name.size() - 3); ++i) {
            if ((d_attributes[i + name.size()] == '=') && (name == d_attributes.substr(i, name.size()))) {
                auto strt = 1 + (i += name.size() + 1);
                for (char q = d_attributes[i]; d_attributes[++i] != q;);
                return d_attributes.substr(strt, i - strt);
            }
            else if (d_attributes[i] == '=')
                for (char q = d_attributes[++i]; d_attributes[++i] != q;);
        }
        return "";
    }
    
    /**
     * @brief Retrieves an attribute value by index.
     *
     * Returns the attribute value at the specified index 'n' from the XML element's attributes.
     *
     * @param n The index of the attribute to retrieve.
     * @return  The attribute value or an empty string if the index is out of bounds.
     */
    std::string const attribute(std::size_t const& n) const noexcept {
        std::string r;
        std::size_t i=0;
        std::size_t found = 0;
        for ( ; (i <= d_attributes.size()) && (found != (n+1)); ++i, ++found)
            i = d_attributes.find('=');
        if (found == (n+1)) {
            i = d_attributes.find_first_of("'\"", i);
            for (std::size_t j = i+1 ; d_attributes[j] != d_attributes[i]; ++j)
                r.push_back(d_attributes[j]);
        }
        return r;
    }
    
    /**
     * @brief Appends an attribute to the tag.
     *
     * @param attribute The attribute name.
     * @param val The optional attribute value or an empty string.
     * @return A reference to the updated XML_element instance.
     */
    XML_element& add_attribute(std::string const& attribute, std::string const& val = "") {
        if (d_attributes.size() != 0)
            d_attributes.append(" ");
        d_attributes.append(attribute);
        if (val.size() != 0)
            d_attributes.append("=\"" + val + "\"");
        return *this;
    }
    
    /**
     * @brief Appends an attribute with a numeric value to a tag.
     *
     * @param attribute The attribute name.
     * @param val The arithmetic value.
     * @return A reference to the updated XML_element instance.
     */
    template <typename T> requires std::is_arithmetic_v<T>
    XML_element& add_attribute(std::string const& attribute, T const val) {
        return add_attribute(attribute, std::to_string(val));
    }
    
    /**
     * @brief Appends an attribute with a range of numeric values to a tag.
     *
     * @param attribute The attribute name.
     * @param container The range of arithmetic values.
     * @return A reference to the updated XML_element instance.
     */
    template <typename T> requires (requires (T c) { std::to_string(*c.begin()); })
    XML_element& add_attribute(std::string const& attribute, T const container) {
        std::string values;
        for (auto it = container.begin(); it != container.end(); ++it) {
            values.append(std::to_string(*it));
            if (it + 1 != container.end())
                values.append(" ");
        }
        return add_attribute(attribute, values);
    }
    
    /**
     * @brief Appends an element.
     *
     * @param elem The element to be added.
     * @return A reference to the updated XML_element instance.
     */
    XML_element& add_element(std::string const& elem) {
        static_cast<std::string&>(*this) = *this + elem;
        d_param = f_find_all_params(std::string(*this));
        return *this;
    }

    /**
     * @brief Appends a range of arithmetic elements.
     *
     * @param container The range of elements to be added.
     * @return A reference to the updated XML_element instance.
     */
    template <typename T> requires (requires (T c) { std::to_string(*c.begin()); })
    XML_element& add_element(std::string const& tag, T const container) {
        std::string s = *this + '<' + tag + '>';
        for (auto it = container.begin(); it != container.end(); ++it)
            s = s + ' ' + std::to_string(*it);
        static_cast<std::string&>(*this) = s + " </" + tag + '>';;
        return *this;
    }
    
    /**
     * @brief Returns the encompassing XML string.
     * @return An XML string, tagged with the tag (and tag attributes) of the XML_element object.
     */
    std::string XML() const noexcept {
        std::string xml = '<' + d_tag;
        if (d_attributes.size() != 0)
            xml = xml + ' ' + d_attributes;
        if (d_param.size() == 0)
            xml = xml + "/>";
        else
            xml = xml + '>' + *this + "</" + d_tag + '>';
        return xml;
    }
    
    /**
     * @brief Retrieves the tag name of the XML_element.
     *
     * Returns the tag name of the XML_element that was used to extract it from the XML data.
     *
     * @return The tag name of the XML element or an empty string if no tag was specified.
     */
    std::string const& tag() const noexcept {return d_tag;}
 
    /**
     * @brief Retrieves an XML element by tag name.
     *
     * Returns the first XML element with the specified 'tag' as an XML_element.
     *
     * @param tag   The tag name to search for.
     * @return      An XML_element containing the first matching element.
     */
    XML_element const tag(std::string const& tag) const noexcept { return multitag(tag, 1)[0]; }

    /**
     * @brief Retrieves multiple XML elements with the same tag.
     *
     * Returns a vector containing all XML elements with the specified 'tag' found in the parsed data,
     * up to a maximum of 'max_tags', if this is provided. If max_tags is not provided, all
     * XML_elements in the stream are returned.
     *
     * @param tag       The tag name to search for.
     * @param max_tags  The maximum number of elements to retrieve. Default is -1 (no limit).
     * @return          A vector of XML elements with the specified tag.
     */
    std::vector<XML_element> const multitag(std::string const& tag, std::size_t max_tags = static_cast<std::size_t>(-1)) const noexcept {
        std::vector<XML_element> r;
        for (std::size_t i=0; (max_tags != 0) && (i != d_param.size()); ++i)
            if (d_param[i].tag() == tag) {
                r.push_back(d_param[i]);
                --max_tags;
            }
        return r;
    }
    
    /**
     * @brief Retrieves values from an XML element that are defined by the first occurence of a specified tag.
     *
     * Returns a vector containing the values of type 'T' parsed from the XML element with the specified 'tag'.
     * If no matching elements are found, an empty vector is returned. For example:
     * @code{.cpp}
     * std::string xml = "<img> <size> 512  512  </size></img>";
     * std::vector<int> val = XML_element(xml).value<int>("size");
     * assert(val[0] == 512 && val[1] == 512);
     * @endcode
    *
     * @tparam T    The type of values to retrieve.
     * @param tag   The tag name to search for.
     * @return      A vector of values of type 'T'.
     */
    template <typename T>
    std::vector<T> const value(std::string const& tag) const noexcept {
        auto tmp = multitag(tag, 1);
        if (tmp.size() == 0)
            return std::vector<T>();
        else
            return tmp[0];
    }
    
    /**
     * @brief Retrieves values from an XML element that are defined by some or all instances of a specified tag.
     *
     * Returns a vector of vectors containing the values of type 'T' parsed from the XML element with the specified 'tag'.
     * If no matching elements are found, an empty vector is returned. For example:
     *
     * @code{.cpp}
     * std::string xml = "<img> <dead_pix> 2  50  </dead_pix><dead_pix> 3 49  </dead_pix></img>"
     * std::vector<std::vector<int>> vals = XML_element(xml).multivalue<int>("dead_pix");
     * assert(vals[0][0] == 2 && vals[1][1] == 49);
     * @endcode
     *
     * @tparam T    The type of values to retrieve.
     * @param tag   The tag name to search for.
     * @param max_tags   The maximum instances of 'tag' to consider; if -1 or not specidfies, all occurences are considered.
     * @return      A vector of vectors of values of type 'T'.
     */
    template <typename T>
    std::vector<std::vector<T>> const multivalue(std::string const& tag, std::size_t max_tags = static_cast<std::size_t>(-1)) const noexcept {
        std::vector<std::vector<T>> r;
        auto tmp = multitag(tag, max_tags);
        for (auto const& t: tmp)
            r.push_back(t);
        return r;
    }
    
    
private:
    std::vector<s_param> d_param;
    std::string d_tag;
    bool d_tag_found = false;
    std::string d_attributes;
        
    std::string f_get_attributes(std::istream& istr) {
        if (d_tag == "")
            return "";
        f_find_tag(istr, d_tag);
        if (d_tag_found) {
            std::string r = std::string(f_read_upto(istr, '>'), d_tag.size() + 1);
            if (r.size() >= 1)
                r.pop_back();
            if ((r.size() > 1) && (r.back() == '/'))
                r.pop_back();
            return r;
        }
        return "";
    }
    
    void f_find_tag(std::istream& istr, std::string const& tag) {
        static auto const all = std::numeric_limits<std::streamsize>::max();
        for (istr.ignore(all, '<'); istr.good() && !f_next_is(istr, tag); istr.ignore(all, '<')) {
            if (f_next_is(istr, "![CDATA["))
                for (f_read_upto(istr, ']'); istr.good() && !f_next_is(istr, "]>"); f_read_upto(istr, ']'));
            else if (f_next_is(istr, "!--"))
                for (f_read_upto(istr, '-'); istr.good() && !f_next_is(istr, "->"); f_read_upto(istr, '-'));
        }
        if (istr.good())
            d_tag_found = true;
    }
    
    std::vector<s_param> f_find_comments_and_data(std::string const& s) {
        std::vector<s_param> r;
        for (s_param p = f_find_interval(s, "<!--", "-->"); p.from  != std::string::npos; p = f_find_interval(s, "<!--", "-->", p.to()))
            r.push_back(p);
        for (s_param p = f_find_interval(s, "<![CDATA[", "]]>"); p.from  != std::string::npos; p = f_find_interval(s, "<![CDATA[", "]]>", p.to()))
            r.push_back(p);
        std::sort(r.begin(), r.end(), [](s_param const& left, s_param const& right) {return left.from < right.from;});
        for (std::size_t i = 0; i!=r.size(); ++i)
            for (std::size_t j = i+1; (j!=r.size()) && (r[j].from < (r[i].from + r[i].size)); ++j)
                r.erase(r.begin() + static_cast<std::ptrdiff_t>(j--));
        return r;
    }
    
    std::vector<s_param> f_find_all_params(std::string const& s) {
        std::vector<s_param> skip = f_find_comments_and_data(s);
        std::vector<s_param> r;
        for (s_param param(*this, f_ignore(s, 0, skip), 1); param.from < s.size(); param.from = f_ignore(s, param.to(), skip)) {
            if (s[param.from] != '<') {
                for (param.size = 1; !f_is_white(s[param.to()]) && (s[param.to()] != '<'); ++param.size);
                r.push_back(param);
            }
            else {
                param.size = s.find('>', param.from) + 1 - param.from;
                if (s[param.to() - 2] == '/')
                    r.push_back(param);
                else {
                    std::size_t end_tag_size = s.find_first_of(" >", param.from) - param.from - 1;
                    std::string const end_tag = "</" + s.substr(param.from + 1, end_tag_size) + '>';
                    param.size = s.find(end_tag, param.to()) - param.from + end_tag.size();
                    for (auto& ignore: skip)
                        if ((param.to() > ignore.from) && (param.to() < ignore.to()))
                            param.size = s.find(end_tag, param.to()) - param.from;
                    r.push_back(param);
                }
            }
        }
        r.insert(r.begin(), skip.begin(), skip.end());
        std::sort(r.begin(), r.end(), [](s_param const& left, s_param const& right) {return left.from < right.from;});
        return r;
    }
    
    s_param f_find_interval(std::string const& s, std::string const& from, std::string const& to, std::size_t pos = 0) {
        std::size_t param_start = s.find(from, pos);
        if (param_start == std::string::npos)
            return s_param(*this, param_start, 0);
        else
            return s_param(*this, param_start, s.find(to, param_start + from.size()) - param_start + to.size());
    }
    
    std::size_t f_ignore(std::string const& s, std::size_t pos, std::vector<s_param> skip = std::vector<s_param>()) {
        std::size_t start = pos;
        std::size_t i = 0;
        do {
            pos = start;
            while (f_is_white(s[start]) && (start < s.size()))
                ++start;
            for ( ; (i != skip.size()) && (start >= skip[i].from); ++i)
                if ((start >= skip[i].from) && (start < skip[i].to()))
                    start = skip[i].to();
        } while ((pos != start) && (start < s.size()));
        return start;
    }
    
    std::string f_read_upto(std::istream& istr, char const c) {
        std::string r;
        int ch;
        while (istr.good() && (ch = istr.get()) != std::char_traits<char>::eof()) {
            r.push_back(static_cast<char>(ch));
            if (ch == c) break;
        }
        return r;
    }
    
    std::string f_read_upto_any_of(std::istream& istr, std::string const& stop) {
        std::string r;
        int ch;
        while (istr.good() && (ch = istr.get()) != std::char_traits<char>::eof()) {
            if (stop.find(static_cast<char>(ch)) != std::string::npos) break;  // ✅ Safe conversion
            r.push_back(static_cast<char>(ch));
        }
        return r;
    }
    
    std::string f_read_element(std::istream& istr, std::string const& tag) {
        std::string element = f_read_upto(istr, '<');
        for (; !f_next_is(istr, '/' + tag + '>'); element += f_read_upto(istr, '<')) {
            if (f_next_is(istr, "![CDATA["))
                for (element += f_read_upto(istr, '>'); !element.compare(element.size() - 3, 3, "]]>"); element += f_read_upto(istr, '>'));
            else if (f_next_is(istr, "!--"))
                for (element += f_read_upto(istr, '>'); !element.compare(element.size() - 3, 3,"-->"); element += f_read_upto(istr, '>'));
        }
        element.pop_back();
        return element;
    }
    
    bool f_is_white(char const c) const noexcept {return (c == 0x20) || (c==0x09) || (c==0x0D) || (c==0x0A);}
    
    bool f_next_is(std::istream& istr, std::string const& s) {
        std::string tst(s.size(), 0);
        if (istr.read(&tst[0], static_cast<std::ptrdiff_t>(s.size()))) {
            istr.seekg(-std::ssize(s), std::ios_base::cur);
            return d_tag_found = tst == s;
        }
        return d_tag_found = false; // Unable to read from the stream, so it's not a match
    }
    
    bool f_prev_is(std::istream& istr, std::string const& s) {
        std::string tst(s.size(), 0);
        istr.seekg(-std::ssize(s), std::ios_base::cur);
        istr.read(&tst[0], static_cast<std::ptrdiff_t>(s.size()));
        return tst == s;
    }
    
    template <typename T>
    operator std::vector<T>() const {
        std::vector<T> r;
        std::istringstream str(std::string(*this));
        for (T tmp; (str >> tmp).good(); ) r.push_back(tmp);
        return r;
    }
};

}

#endif /* XML_element_h */
