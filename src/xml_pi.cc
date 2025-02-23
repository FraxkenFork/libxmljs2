#include <node.h>

#include <cstring>

#include "libxmljs.h"

#include "xml_pi.h"
#include "xml_document.h"
#include "xml_attribute.h"
#include "xml_xpath_context.h"

using namespace v8;

namespace libxmljs
{

Nan::Persistent<FunctionTemplate> XmlProcessingInstruction::constructor_template;

// doc, content
NAN_METHOD(XmlProcessingInstruction::New)
{
  Nan::HandleScope scope;

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0)
  {
    return info.GetReturnValue().Set(info.Holder());
  }

  XmlDocument *document = Nan::ObjectWrap::Unwrap<XmlDocument>(Nan::To<Object>(info[0]).ToLocalChecked());
  assert(document);

  Nan::Utf8String name(info[1]);

  Local<Value> contentOpt;
  if (info[2]->IsString())
  {
    contentOpt = info[2];
  }
  Nan::Utf8String contentRaw(contentOpt);
  const char *content = (contentRaw.length()) ? *contentRaw : NULL;

  xmlNode *pi = xmlNewDocPI(document->xml_obj, (const xmlChar *)*name, (xmlChar *)content);

  XmlProcessingInstruction *processing_instruction = new XmlProcessingInstruction(pi);
  pi->_private = processing_instruction;
  processing_instruction->Wrap(info.Holder());

  // this prevents the document from going away
  Nan::Set(info.Holder(), Nan::New<String>("document").ToLocalChecked(), info[0]).Check();

  return info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(XmlProcessingInstruction::Name)
{
  Nan::HandleScope scope;
  XmlProcessingInstruction *processing_instruction = Nan::ObjectWrap::Unwrap<XmlProcessingInstruction>(info.Holder());
  assert(processing_instruction);

  if (info.Length() == 0)
    return info.GetReturnValue().Set(processing_instruction->get_name());

  Nan::Utf8String name(Nan::To<String>(info[0]).ToLocalChecked());
  processing_instruction->set_name(*name);
  return info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(XmlProcessingInstruction::Text)
{
  Nan::HandleScope scope;
  XmlProcessingInstruction *processing_instruction = Nan::ObjectWrap::Unwrap<XmlProcessingInstruction>(info.Holder());
  assert(processing_instruction);

  if (info.Length() == 0)
  {
    return info.GetReturnValue().Set(processing_instruction->get_content());
  }
  else
  {
    processing_instruction->set_content(*Nan::Utf8String(info[0]));
  }

  return info.GetReturnValue().Set(info.Holder());
}

void XmlProcessingInstruction::set_name(const char *name)
{
  xmlNodeSetName(xml_obj, (const xmlChar *)name);
}

Local<Value>
XmlProcessingInstruction::get_name()
{
  Nan::EscapableHandleScope scope;
  if (xml_obj->name)
    return scope.Escape(Nan::New<String>((const char *)xml_obj->name).ToLocalChecked());
  else
    return scope.Escape(Nan::Undefined());
}

void XmlProcessingInstruction::set_content(const char *content)
{
  xmlNodeSetContent(xml_obj, (xmlChar *)content);
}

Local<Value>
XmlProcessingInstruction::get_content()
{
  Nan::EscapableHandleScope scope;
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content)
  {
    Local<String> ret_content =
        Nan::New<String>((const char *)content).ToLocalChecked();
    xmlFree(content);
    return scope.Escape(ret_content);
  }

  return scope.Escape(Nan::New<String>("").ToLocalChecked());
}

Local<Object>
XmlProcessingInstruction::New(xmlNode *node)
{
  Nan::EscapableHandleScope scope;
  if (node->_private)
  {
    return scope.Escape(static_cast<XmlNode *>(node->_private)->handle());
  }

  XmlProcessingInstruction *processing_instruction = new XmlProcessingInstruction(node);
  Local<Object> obj = Nan::NewInstance(Nan::GetFunction(Nan::New(constructor_template)).ToLocalChecked()).ToLocalChecked();
  processing_instruction->Wrap(obj);
  return scope.Escape(obj);
}

XmlProcessingInstruction::XmlProcessingInstruction(xmlNode *node)
    : XmlNode(node)
{
}

void XmlProcessingInstruction::Initialize(Local<Object> target)
{
  Nan::HandleScope scope;
  Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(static_cast<NAN_METHOD((*))>(New));
  t->Inherit(Nan::New(XmlNode::constructor_template));
  t->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template.Reset(t);

  Nan::SetPrototypeMethod(t,
                          "name",
                          XmlProcessingInstruction::Name);

  Nan::SetPrototypeMethod(t,
                          "text",
                          XmlProcessingInstruction::Text);

  Nan::Set(target, Nan::New<String>("ProcessingInstruction").ToLocalChecked(),
           Nan::GetFunction(t).ToLocalChecked());
}

} // namespace libxmljs
