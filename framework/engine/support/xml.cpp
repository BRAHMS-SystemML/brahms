/*
________________________________________________________________

	This file is part of BRAHMS
	Copyright (C) 2007 Ben Mitchinson
	URL: http://brahms.sourceforge.net

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
________________________________________________________________

	Subversion Repository Information (automatically updated on commit)

	$Id:: xml.cpp 2309 2009-11-06 00:49:32Z benjmitch          $
	$Rev:: 2309                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-06 00:49:32 +0000 (Fri, 06 Nov 2009)       $
________________________________________________________________

*/



#include "support.h"




#define XMLNODE_CHECK_NOT_READONLY { if (control && (control->readonly)) ferr << E_XML << "attempt to change read-only " << nodeNoun(); }



////////////////	NAMESPACE

namespace brahms
{
	namespace xml
	{





	////////////////	HELPER FUNCTIONS

		void validateName(XMLNode* node, const char* text, const char* what)
		{
			if (!text) return;
			if (!(*text))
				ferr << E_XML_PARSE << "invalid " << what << " on XMLNode \"" << node->nodeName() << "\" (empty string)";
			if (!(isalpha(*text) || *text == '_'))
				ferr << E_XML_PARSE << "invalid " << what << " on XMLNode \"" << node->nodeName() << "\" (must start with alpha or underscore)";
			while(*text)
			{
				if (!(isalnum(*text) || *text == '_'))
					ferr << E_XML_PARSE << "invalid " << what << " on XMLNode \"" << node->nodeName() << "\" (all characters must be alphanumeric or underscore)";
				text++;
			}
		}

		void readStreamIntoString(std::string& text, std::istream& src)
		{
			// get the file size
			src.seekg(0, ios::end);
			size_t fileSize = src.tellg();
			src.seekg(0, ios::beg);

			//	empty file is an error
			if (!fileSize)
				ferr << E_XML_PARSE << "stream was empty";

			// allocate enough memory
			text.resize(fileSize);

			// read the content of the file
			src.read(&text[0], fileSize);

			//	get pointer to start of file
			if (!text.length()) ferr << E_XML_PARSE << "stream was empty";
		}

		string parseat(const char* text, const char* next)
		{
			//	assume this character was the first of the line
			const char* begin = next;

			//	whilst the previous character is not a newline or SOF, reassess that assumption
			while((begin > text) && (*(begin-1) != '\n'))
				begin--;

			//	"begin" is now first character of line
			INT32 column = next - begin + 1;

			//	now count how many lines came before
			INT32 index = 1;
			const char* scan = begin;
			while(scan > text)
			{
				scan--;
				if (*scan == '\n') index++;
			}

			//	start constructing err msg
			stringstream at;
			at << ", at line " << index << " column " << column << ":\n\n\"";

			//	number of context characters
			INT32 N = 16;

			//	wind back to beginning of line or N characters, whichever comes first
			INT32 before = next - begin;
			if (before > N)
			{
				at << "...";
				begin = next - N + 3;
				before = N;
			}

			//	wind forward to end of line or N characters, whichever comes first
			const char* end = next;
			while(true)
			{
				if (!*end) break; // EOF
				if (*end == '\n') break; // EOL
				if ((end - next) == N)
				{
					end -= 3;
					break;
				}
				end++;
			}

			//	add context
			string context;
			context.append(begin, end-begin);
			for (string::iterator i=context.begin(); i!=context.end(); i++)
				if (*i == '\t') *i = ' ';
			at << context;

			//	add termination
			before++;
			if (*end && *end != '\n') at << "...";
			at << "\"";
			//if (*end == 0) at << " (end of file)"; // don't think we need this because the err msg always mentions EOF if it is relevant
			at << "\n";

			//	add indicator
			for (INT32 i=0; i<before; i++) at << " ";
			at << "^\n";

			//	ok
			return at.str();
		}

		void xmlparse(XMLNode* node, const char* text, const char*& next)
		{
			//	text remainder
			string parsedText;
			bool nonWhitespaceTextContent = false;

			//	parse
			while(*next)
			{

				//	switch on character type
				switch(*next)
				{

					//	open tag
					case '<':
					{
						//	advance
						next++;

						//	EOF?
						if (!*next) ferr << E_XML_PARSE << "unexpected end of file in tag" << parseat(text, next);

						//	could be a command tag <?
						if (*next == '?')
						{
							//	advance until we see ?> and then continue
							while(true)
							{
								next++;

								if (!*next) ferr << E_XML_PARSE << "unexpected end of file in command" << parseat(text, next);
								if (*next == '>' && *(next-1) == '?')
								{
									next++;

									break;
								}
							}
							continue;
						}

						//	could be a comment <!
						if (*next == '!')
						{
							//	advance until we see --> and then continue
							while(true)
							{
								next++;

								if (!*next) ferr << E_XML_PARSE << "unexpected end of file in comment" << parseat(text, next);
								if (*next == '>' && *(next-1) == '-' && *(next-2) == '-')
								{
									next++;

									break;
								}
							}
							continue;
						}


						//	what sort of tag?
						bool open = false;
						bool close = false;

						//	could be a close tag
						if (*next == '/')
						{
							next++;
							close = true;
						}
						else open = true;

						//	check here for mixed content to get a good err msg
						if (open && nonWhitespaceTextContent)
						{
							next--; // retire to '<' character (we've moved on to first char of tag name)
							ferr << E_XML_PARSE << "mixed content not handled by this parser" << parseat(text, next);
						}

						//	must be a normal tag
						if (!brahms::istokenstart(*next))
							ferr << E_XML_PARSE << "not tag name start character" << parseat(text, next);

						//	start collecting tag name with that first character
						string parsedTagName;
						parsedTagName += *next;
						next++;

						//	get rest of tag name
						while(brahms::istokencontinue(*next))
						{
							parsedTagName += *next;
							next++;
						}

						//	close tag must end there
						if (close && (*next != '>'))
							ferr << E_XML_PARSE << "expected '>'" << parseat(text, next);

						//	now look for attributes
						vector<XMLAttr> parsedAttributes;
						while(true)
						{
							//	after tagName we can have whitespace...
							if (*next == ' ')
							{
								//	suck up whitespace
								while(*next == ' ') next++;

								//	get attribute name
								if (!brahms::istokenstart(*next))
									ferr << E_XML_PARSE << "not attribute name start character" << parseat(text, next);

								//	get rest of attr name
								string attrkey;
								while(brahms::istokencontinue(*next))
								{
									attrkey += *next;
									next++;
									continue;
								}

								//	get attr value
								if (*next != '=') ferr << E_XML_PARSE << "expected '='" << parseat(text, next);
								next++;
								if (*next != '\"') ferr << E_XML_PARSE << "expected '\"'" << parseat(text, next);
								next++;
								string attrvalue;
								while(true)
								{
									if (!*next) ferr << E_XML_PARSE << "unexpected end of file in attribute value" << parseat(text, next);
									if (*next == '\"') break;
									attrvalue += *next;
									next++;
								}
								next++;

								XMLAttr attr;
								attr.name = attrkey;
								attr.value = attrvalue;
								parsedAttributes.push_back(attr);
								continue;
							}

							//	or end of tag...
							if (*next == '>')
							{
								next++;
								break;
							}

							//	or end of tag with closure...
							if (*next == '/' && *(next+1) == '>')
							{
								close = true; // now "open" _and_ "close" are both true!
								next += 2;
								break;
							}

							//	anything else is an error
							ferr << E_XML_PARSE << "unexpected character whilst parsing tag" << parseat(text, next);
						}

						//	if we don't have a tagName yet, we are the root parsing node, and
						//	the document has just begun, with the first tag we read being the document open tag
						if (!node->getObjectName().length())
						{
							//	it must be an open tag
							if (!open) ferr << E_XML_PARSE << "document invalid (bad root tag)" << parseat(text, next);

							//	take its name and attributes to ourselves
							node->setObjectName(parsedTagName);
							node->element.attributes = parsedAttributes;

							//	if we were also closed, that's the end of the document right there!
							if (close) return;

							//	if we were not closed, we can continue parsing content ourselves
							continue;
						}

						//	if it's an open tag, we need to create a new child
						if (open)
						{
							//	create child using tag name and attributes
							XMLNode* child = node->appendChild(new XMLNode(parsedTagName.c_str()));
							child->element.attributes = parsedAttributes;

							//	if it was also closed, we can carry straight on parsing
							if (close) continue;

							//	otherwise, we get the new child to continue the parse until its close tag...
							xmlparse(child, text, next);

							//	...and _then_ we carry straight on
							continue;
						}

						//	otherwise, it _must_ be a close tag!
						if (!close) ferr << E_INTERNAL << "XML parser internal error (how can this not be a close tag?)" << parseat(text, next);

						//	mixed content illegal
						if (parsedText.length())
						{
							if (node->element.children.size())
							{
								//	try to interpret the text as all whitespace
								const char* t = parsedText.c_str();
								while(*t)
								{
									//	all spaces is considered just "padding"
									if ( !isspace(*t) )
										ferr << E_XML_PARSE << "mixed content not handled by this parser" << parseat(text, next);
									t++;
								}
								node->element.text = "";
							}
							else node->element.text = parsedText;
						}

						//	and it must be _our_ expected close tag
						if (parsedTagName != node->getObjectName())
							ferr << E_XML_PARSE << node->nodeNoun() << " closed by </" << parsedTagName << ">" << parseat(text, next);

						//	so now we can hand control back to the parent parser
						return;
					}



					//	entity
					case '&':
					{
						//	check here for mixed content to get a good err msg
						if (node->element.children.size())
							ferr << E_XML_PARSE << "mixed content not handled by this parser" << parseat(text, next);
						nonWhitespaceTextContent = true;

						//	consume XML entity
						string entity;
						next++;
						while(true)
						{
							if (!*next) ferr << E_XML_PARSE << "unexpected end of file in XML entity" << parseat(text, next);

							if (*next == ';')
							{
								if (entity == "amp")
								{
									parsedText += "&";
									next++;
									break;
								}

								if (entity == "apos")
								{
									parsedText += "'";
									next++;
									break;
								}

								if (entity == "quot")
								{
									parsedText += "\"";
									next++;
									break;
								}

								if (entity == "lt")
								{
									parsedText += "<";
									next++;
									break;
								}

								if (entity == "gt")
								{
									parsedText += ">";
									next++;
									break;
								}

								ferr << E_XML_PARSE << "unrecognised XML entity" << parseat(text, next);
							}
							entity += *next;
							next++;
						}

						break;
					}



					//	white space
					case ' ': case '\t': case '\r': case '\n':
					{
						parsedText += *next;
						next++;
						break;
					}



					//	other character
					default:
					{
						//	check here for mixed content to get a good err msg
						if (node->element.children.size())
							ferr << E_XML_PARSE << "mixed content not handled by this parser" << parseat(text, next);
						nonWhitespaceTextContent = true;

						parsedText += *next;
						next++;
						break;
					}

				}

			}

			//	should never reach EOF without closing a tag
			ferr << E_XML_PARSE << "unexpected end of file whilst tag open" << parseat(text, next);
		}

		void inline xmlsafe(std::ostream& dst, const char* text)
		{
			//	search for dodgy characters
			const char* p = text;
			UINT32 c = 0;

			while(p[c])
			{
				switch(p[c])
				{
				case '<': dst.write(p, c); dst << "&lt;";  p += c + 1; c=0; break;
				case '>': dst.write(p, c); dst << "&gt;";  p += c + 1; c=0; break;
				case '&': dst.write(p, c); dst << "&amp;";  p += c + 1; c=0; break;
				case '\'': dst.write(p, c); dst << "&apos;";  p += c + 1; c=0; break;
				case '"': dst.write(p, c); dst << "&quot;";  p += c + 1; c=0; break;
				default: c++;
				}
			}

			//	add the last
			dst.write(p, c);
		}







	////////////////	XML CLASS IMPLEMENTATION

		string XMLNode::nodeNoun(bool recursed) const
		{
			string noun = parent ? parent->nodeNoun(true) : "";
			if (!recursed) noun = "XML node " + noun;
			noun += "<" + getObjectName() + ">";
			return noun;
		}

		XMLNode::XMLNode(const char* tagName, const char* text)
			:
			RegisteredObject(CT_XMLNODE, "")
		{
			parent = NULL;
			userData = 0;
			control = NULL;
			nodeName(tagName);
			nodeText(text);
		}

		XMLNode::XMLNode(std::istream& src)
			:
			RegisteredObject(CT_XMLNODE, "")
		{
			parent = NULL;
			userData = 0;
			control = NULL;
			parse(src);
			validateName(this, getObjectName().c_str(), "root node name from parsed document");
		}

		/*

			WARNING!

			If updating the copy ctor or the assignment operator,
			consider also updating cloneNode() below.

		*/

		XMLNode::XMLNode(const XMLNode& src)
			:
			RegisteredObject(CT_XMLNODE, src.getObjectName())
		{
			//	copy all children
			for (UINT32 c=0; c<src.element.children.size(); c++)
				element.children.push_back(new XMLNode(*src.element.children[c]));

			element.attributes = src.element.attributes;
			element.text = src.element.text;
			userData = src.userData;
			control = src.control;

			//	a copy is not automatically attached to the same parent
			parent = NULL;
		}

		XMLNode& XMLNode::operator=(const XMLNode& src)
		{
			if (this == &src) return *this; // handle self-assignment gracefully

			//	delete all existing children
			removeChildren();

			//	copy all children
			for (UINT32 c=0; c<src.element.children.size(); c++)
				element.children.push_back(new XMLNode(*src.element.children[c]));

			element.attributes = src.element.attributes;
			element.text = src.element.text;
			userData = src.userData;
			control = src.control;

			//	on assignment, our parent does not change, just our content
			//parent = ...

			return *this;
		}

		XMLNode::~XMLNode()
		{
			//	clear control pointer, so we don't throw an error at this attempt to modify the object
			control = NULL;

			//	delete all existing children
			removeChildren();
		}








	////////////////	W3C "NODE" INTERFACE

		const char* XMLNode::nodeName() const
		{
			return getObjectName().c_str();
		}

		void XMLNode::nodeName(const char* tagName)
		{
			XMLNODE_CHECK_NOT_READONLY;

			validateName(this, tagName, "node name");
			setObjectName(tagName ? tagName : "");
		}

		XMLNode* XMLNode::parentNode() const
		{
			return parent;
		}

		const XMLNodeList* XMLNode::childNodes() const
		{
			return &element.children;
		}

		XMLNode* XMLNode::firstChild()
		{
			if (element.children.size()) return element.children[0];
			else return NULL;
		}

		XMLNode* XMLNode::lastChild()
		{
			if (element.children.size()) return element.children[element.children.size() - 1];
			else return NULL;
		}

		XMLNode* XMLNode::previousSibling()
		{
			if (parent)
			{
				//	get parent's children list
				const XMLNodeList* siblings = parent->childNodes();

				//	find my index in parent
				UINT32 index;
				for (index=0; index<siblings->size(); index++)
					if ((*siblings)[index] == this) break;
				if (index == siblings->size())
					ferr << E_INTERNAL << nodeNoun() << " (0x" << hex << this << ") not found in own parent";

				//	return previous sibling if there is one or drop through to return NULL
				if (index > 0) return (*siblings)[index - 1];
			}
			return NULL;
		}

		XMLNode* XMLNode::nextSibling()
		{
			if (parent)
			{
				//	get parent's children list
				const XMLNodeList* siblings = parent->childNodes();

				//	find my index in parent
				UINT32 index;
				for (index=0; index<siblings->size(); index++)
					if ((*siblings)[index] == this) break;
				if (index == siblings->size())
					ferr << E_INTERNAL << nodeNoun() << " (0x" << hex << this << ") not found in own parent";

				//	return next sibling if there is one or drop through to return NULL
				if (index < (siblings->size() - 1)) return (*siblings)[index + 1];
			}
			return NULL;
		}

		XMLAttrList* XMLNode::attributes()
		{
			return &element.attributes;
		}

		XMLNode* XMLNode::insertBefore(XMLNode* newChild, XMLNode* refChild)
		{
			XMLNODE_CHECK_NOT_READONLY;

			if (refChild)
			{
				for (UINT32 index=0; index<element.children.size(); index++)
				{
					if (element.children[index] == refChild)
					{
						//	we become the new child's parent
						newChild->parent = this;

						element.children.insert(element.children.begin() + index, newChild);
						return newChild;
					}
				}
				ferr << E_XML << "reference node not found in insertBefore()";
			}

			//	we become the new child's parent
			newChild->parent = this;

			//	else just append
			element.children.push_back(newChild);
			return newChild;
		}

		XMLNode* XMLNode::replaceChild(XMLNode* newChild, XMLNode* oldChild)
		{
			XMLNODE_CHECK_NOT_READONLY;

			for (UINT32 index=0; index<element.children.size(); index++)
			{
				if (element.children[index] == oldChild)
				{
					//	we swap to become in charge of the new one
					element.children[index] = newChild;

					//	and become the new child's parent
					newChild->parent = this;

					//	whilst setting the old child to be an orphan
					oldChild->parent = NULL;

					//	and returning it to belong to the caller
					return oldChild; // see W3C rec "Return Value: The node replaced."
				}
			}
			ferr << E_XML << "child not found in replaceChild()";

			//	avoid debug-mode warning...
			return 0;
		}

		XMLNode* XMLNode::removeChild(XMLNode* oldChild)
		{
			XMLNODE_CHECK_NOT_READONLY;

			for (UINT32 index=0; index<element.children.size(); index++)
			{
				if (element.children[index] == oldChild)
				{
					element.children.erase(element.children.begin() + index);
					return oldChild;
				}
			}
			ferr << E_XML << "child not found in removeChild()";

			//	avoid debug-mode warning...
			return 0;
		}

		void XMLNode::deleteChild(XMLNode* oldChild)
		{
			XMLNODE_CHECK_NOT_READONLY;

			for (UINT32 index=0; index<element.children.size(); index++)
			{
				if (element.children[index] == oldChild)
				{
					delete element.children[index];
					element.children.erase(element.children.begin() + index);
					return;
				}
			}
			ferr << E_XML << "child not found in deleteChild()";
		}

		XMLNode* XMLNode::appendChild(XMLNode* newChild)
		{
			XMLNODE_CHECK_NOT_READONLY;

			//	must not have a parent
			if (newChild->parent) throw E_INVALID_ARG;

			//	we become the new child's parent
			newChild->parent = this;

			//	and it inherits our control object
			newChild->control = control;

			element.children.push_back(newChild);
			return newChild;
		}

		bool XMLNode::hasChildNodes() const
		{
			return element.children.size();
		}

		XMLNode* XMLNode::cloneNode(bool deep) const
		{
			if (deep) return new XMLNode(*this);

			//	do we ever use this, really? it's a bit risky because i wonder what becomes of the "control" member...
			ferr << E_INTERNAL << "shallow copy XML";

			//	shallow copy, don't copy children
			XMLNode* shallowCopy = new XMLNode(getObjectName().c_str());
			shallowCopy->element.attributes = element.attributes;
			// we don't take the text! (see the W3C def of cloneNode()) shallowCopy->element.text = src.element.text;
			shallowCopy->userData = userData; // pretty arbitrary whether we take this, since we can't know what it is for

			//	a copy is not automatically attached to the same parent
			shallowCopy->parent = NULL;

			return shallowCopy;
		}

		bool XMLNode::hasAttributes() const
		{
			return element.attributes.size();
		}

		bool XMLNode::isSameNode(const XMLNode* node) const
		{
			return this == node;
		}

		INT32 XMLNode::getUserData() const
		{
			return userData;
		}

		void XMLNode::setUserData(INT32 data)
		{
			XMLNODE_CHECK_NOT_READONLY;

			userData = data;
		}








	////////////////	W3C "ELEMENT" INTERFACE

		const char* XMLNode::getAttribute(const char* name) const
		{
			for (UINT32 index=0; index<element.attributes.size(); index++)
			{
				if (element.attributes[index].name == name)
					return element.attributes[index].value.c_str();
			}

			ferr << E_NOT_FOUND << "attribute \"" << name << "\" not found on " << nodeNoun();

			//	avoid debug-mode warning...
			return 0;
		}

		const char* XMLNode::getAttributeOrNull(const char* name) const
		{
			for (UINT32 index=0; index<element.attributes.size(); index++)
			{
				if (element.attributes[index].name == name)
					return element.attributes[index].value.c_str();
			}

			return NULL;
		}

		void XMLNode::setAttribute(const char* name, const char* value)
		{
			XMLNODE_CHECK_NOT_READONLY;

			validateName(this, name, "attribute name");

			for (UINT32 index=0; index<element.attributes.size(); index++)
			{
				if (element.attributes[index].name == name)
				{
					element.attributes[index].value = value;
					return;
				}
			}

			XMLAttr attr;
			attr.name = name ? name : "";
			attr.value = value ? value : "";
			element.attributes.push_back(attr);
		}

		void XMLNode::removeAttribute(const char* name)
		{
			XMLNODE_CHECK_NOT_READONLY;

			for (UINT32 index=0; index<element.attributes.size(); index++)
			{
				if (element.attributes[index].name == name)
				{
					element.attributes.erase(element.attributes.begin() + index);
					return;
				}
			}

			//	if not found, just return silently (W3C rec)
		}

		XMLNodeList XMLNode::getElementsByTagName(const char* tagName) const
		{
			//	special case (see the DOM specification)
			if (!tagName) return element.children;

			XMLNodeList list;
			for (UINT32 index=0; index<element.children.size(); index++)
				if (element.children[index]->getObjectName() == tagName) list.push_back(element.children[index]);
			return list;
		}

		bool XMLNode::hasAttribute(const char* name) const
		{
			for (UINT32 index=0; index<element.attributes.size(); index++)
			{
				if (element.attributes[index].name == name)
					return true;
			}
			return false;
		}





	////////////////	EXTENSIONS TO THE W3C INTERFACES

		bool XMLNode::hasChild(const char* child) const
		{
			if (!child) ferr << E_XML << "NULL passed to hasChild()";

			for (UINT32 index=0; index<element.children.size(); index++)
			{
				if (element.children[index]->getObjectName() == child)
					return true;
			}

			return false;
		}

		XMLNode* XMLNode::getChild(const char* tagName, UINT32 p_index)
		{
			//	there's no easy way to do this with the DOM interface, which treats
			//	elements as an unordered set rather than as an associative array.
			//	this function and hasChild(), together, give the associateive array
			//	functionality to the unordered set provided by the DOM.

			//if (!tagName) ferr << E_XML << "NULL passed to getChild()";

			UINT32 count = 0;
			for (UINT32 index=0; index<element.children.size(); index++)
			{
				if ((!tagName) || (element.children[index]->getObjectName() == tagName))
				{
					if (count == p_index) return element.children[index];
					count++;
				}
			}

			if (p_index) ferr << E_NOT_FOUND << "expected " << nodeNoun() << " to have child <" << tagName << "[" << p_index << "]>";
			ferr << E_NOT_FOUND << "expected " << nodeNoun() << " to have child <" << tagName << ">";

			//	avoid debug-mode warning...
			return 0;
		}

		XMLNode* XMLNode::getChildOrNull(const char* nodeName, UINT32 p_index)
		{
			if (nodeName)
			{
				//	return p_index'th node with given node name
				UINT32 count = 0;
				for (UINT32 index=0; index<element.children.size(); index++)
				{
					if (element.children[index]->getObjectName() == nodeName)
					{
						if (count == p_index) return element.children[index];
						count++;
					}
				}
			}

			else
			{
				//	return p_index'th node with *any* node name
				if (p_index < element.children.size())
					return element.children[p_index];
			}

			return NULL;
		}

		void XMLNode::removeAttributes()
		{
			XMLNODE_CHECK_NOT_READONLY;

			element.attributes.clear();
		}

		void XMLNode::removeChildren()
		{
			XMLNODE_CHECK_NOT_READONLY;

			//	just a shortcut to delete all existing children
			for (UINT32 index=0; index<element.children.size(); index++)
				delete element.children[index];
			element.children.clear();
		}

		void XMLNode::clear()
		{
			XMLNODE_CHECK_NOT_READONLY;

			//	note nodeName is not cleared!
			removeChildren();
			removeAttributes();
			element.text = "";
		}








	////////////////	IMPLEMENTATION-SPECIFIC INTERFACE

		bool XMLNode::hasNodeText() const
		{
			return element.text.length();
		}

		const char* XMLNode::nodeText() const
		{
			return element.text.c_str();
		}

		void XMLNode::nodeText(const char* text)
		{
			XMLNODE_CHECK_NOT_READONLY;

			element.text = text ? text : "";
		}

		void XMLNode::parse(std::istream& src)
		{
			XMLNODE_CHECK_NOT_READONLY;

			//	for now, just cheat by reading it into a string, and parsing from that
			string text;
			readStreamIntoString(text, src);
			parse(text.c_str());
		}

		const char* XMLNode::parse(const char* text)
		{
			XMLNODE_CHECK_NOT_READONLY;

			const char* next = text;
			xmlparse(this, text, next);
			return next;
		}

		void XMLNode::serialize(std::ostream& dst, UINT32 indent, UINT32 flags, XMLNode* start, XMLNode* stop) const
		{
			/*

				If start is non-NULL, start serializing *after* this node. You can
				do this by working up to its most parent, which should be this node,
				and proceeding from there.

				If stop is non-NULL, stop serializing when you reach that node, so
				don't serialize the stop node.

				These allow us to serialize bits of the XML document at a time, so
				that we can use other mechanisms for serializing individual nodes if
				necessary.

				NOTE: none of the above is yet implemented, but we may need to use
				it when we want to serialize in clever ways. In particular, we may
				wish for a process to be serializing its XML as it goes, since it
				may generate an arbitrarily large file. We can then generate the
				final document by serializing the bits around such pre-serialized
				XML, then streaming in the pre-serialized stuff, then continuing with
				the main document.

			*/

			if (indent)
			{
				for (UINT32 i=1; i<indent; i++)
					dst << "  ";
			}

			//	opening tag
			if (!getObjectName().length())
			{
				if (parent)
					ferr << E_XML << "an XML node (a child of " << parent->nodeNoun(true) << ") had empty name whilst serialize()ing";
				ferr << E_XML << "an orphan XML node had empty name whilst serialize()ing";
			}
			dst << "<" << getObjectName();
			for (UINT32 a=0; a<element.attributes.size(); a++)
			{
				dst << " " << element.attributes[a].name << "=\"";
				xmlsafe(dst, element.attributes[a].value.c_str());
				dst << "\"";
			}

			//	no mixed content: handle text
			if (element.text.length())
			{
				dst << ">";

				xmlsafe(dst, element.text.c_str());

				//	closing tag
				dst << "</" << getObjectName() << ">";
			}

			//	no mixed content: handle children
			else if (element.children.size())
			{
				dst << ">";

				//	new line before children
				if (indent)
				{
					dst << "\n";
				}

				//	children, more indented
				for (UINT32 c=0; c<element.children.size(); c++)
					element.children[c]->serialize(dst, indent ? (indent + 1) : 0, flags, start, stop);

				//	indent for closing tag
				for (UINT32 i=1; i<indent; i++)
					dst << "  ";

				//	closing tag
				dst << "</" << getObjectName() << ">";
			}

			//	no mixed content: no content
			else
			{
				dst << "/>";
			}

			//	new tag is always on new line in TidyXML
			if (indent)
			{
				dst << "\n";
			}
		}

		void XMLNode::setControl(const XMLControl* p_control)
		{
			control = p_control;
		}



////////////////	NAMESPACE

	}
}


