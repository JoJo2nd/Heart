
    # filename: 	hEvent_gen.py	
    
    # Copyright (c) 2012 James Moran
    
    # This software is provided 'as-is', without any express or implied
    # warranty. In no event will the authors be held liable for any damages
    # arising from the use of this software.
    
    # Permission is granted to anyone to use this software for any purpose,
    # including commercial applications, and to alter it and redistribute it
    # freely, subject to the following restrictions:
    
    # 1. The origin of this software must not be misrepresented; you must not
    # claim that you wrote the original software. If you use this software
    # in a product, an acknowledgment in the product documentation would be
    # appreciated but is not required.
    
    # 2. Altered source versions must be plainly marked as such, and must not be
    # misrepresented as being the original software.
    
    # 3. This notice may not be removed or altered from any source
    # distribution.

import argparse
import os.path
from string import Template
    
def outputTxtHeader(outfile, filename):
    outfile.write("/********************************************************************\n");
    outfile.write("    "+filename+"\n");
    outfile.write("    This file is auto generated. DO NOT EDIT. See hEvent_gen.py to\n");
    outfile.write("    change code within this file\n");
    outfile.write("********************************************************************/\n");
    outfile.write("\n")
    outfile.write("template < typename t_Fn >\n")
    outfile.write("class HEART_DLLEXPORT hPublisher;\n")
    
def outputTxtFooter(outfile, filename):
    pass
    
def outputPublisherClassDef(outfile, argcount):
    outfile.write("\n")
    outfile.write("//////////////////////////////////////////////////////////////////////////\n")
    outfile.write("//////////////////////////////////////////////////////////////////////////\n")
    outfile.write("//////////////////////////////////////////////////////////////////////////\n")
    outfile.write("\n")
    temp = Template("typename t_P$itr")
    outfile.write("template < ") #typename t_ty 
    for i in range(0, argcount):
        if i > 0: outfile.write(", ")
        outfile.write(temp.substitute(itr=i+1))
        if i+1 % 10 == 0:
            outfile.write("\n    ")
    outfile.write(">\n")
    outfile.write("class HEART_FORCEDLLEXPORT hPublisher< void(*)(")
    for i in range(0, argcount):
        if i > 0: outfile.write(", ")
        outfile.write(Template("t_P$itr").substitute(itr=i+1))
    outfile.write(") >\n")
    outfile.write("{\n");
    outfile.write("public:\n")
    outfile.write("    typedef hPublisher< void(*)(")
    for i in range(0, argcount):
        if i > 0: outfile.write(", ")
        temp = Template("t_P$itr")
        outfile.write(temp.substitute(itr=i+1))
    outfile.write(") > SelfType;\n")
    for i in range(0, argcount):
        temp = Template("    typedef t_P$itr Param${itr}Type;\n")
        outfile.write(temp.substitute(itr=i+1))
    if argcount == 0:
        outfile.write("    hFUNCTOR_TYPEDEF(void(*)(")
    else:
        outfile.write("    typename hFUNCTOR_TYPEDEF(void(*)(")
    for i in range(0, argcount):
        if i > 0: outfile.write(", ")
        temp = Template("Param${itr}Type")
        outfile.write(temp.substitute(itr=i+1))
    outfile.write("), Delegate);\n")
    outfile.write("    hPublisher() {}\n");
    outfile.write("    hPublisher(const hPublisher& rhs)\n")
    outfile.write("    {\n")
    outfile.write("        // Note that when a event is copied/assigned it becomes two separate events, not two\n")
    outfile.write("        // event instances pointing to the same event i.e.\n")
    outfile.write("        // eventb = eventa\n")
    outfile.write("        // eventb.connect() <- event B will have an extra subcriber that event A doesn't have\n")
    outfile.write("        // eventa.disconnect() <- event A loses a subscriber, but event B still maintains the subscriber\n")
    outfile.write("        rhs.subscribers_.copy_to(&subscribers_);\n")
    outfile.write("    }\n")
    outfile.write("    hPublisher& operator = (const hPublisher& rhs)\n")
    outfile.write("    {\n")
    outfile.write("        // Note that when a event is copied/assigned it becomes two separate events, not two\n")
    outfile.write("        // event instances pointing to the same event i.e.\n")
    outfile.write("        // eventb = eventa\n")
    outfile.write("        // eventb.connect() <- event B will have an extra subcriber that event A doesn't have\n")
    outfile.write("        // eventa.disconnect() <- event A loses a subscriber, but event B still maintains the subscriber\n")
    outfile.write("        rhs.subscribers_.copy_to(&subscribers_);\n")
    outfile.write("        return *this;\n")
    outfile.write("    }\n")
    outfile.write("    ~hPublisher()\n")
    outfile.write("    {\n")
    outfile.write("    }\n")
    # deferred version
    outfile.write("    void operator () (hPublisherContext* ctx")
    for i in range(0, argcount):
        temp = Template(", Param${itr}Type param$itr")
        outfile.write(temp.substitute(itr=i+1))
    outfile.write(")\n")
    outfile.write("    {\n")
    outfile.write("        ParamStruct* params = (ParamStruct*)ctx->pushSignal(\n")
    outfile.write("            sizeof(ParamStruct), \n")
    outfile.write("            hFUNCTOR_BINDMEMBER(hPublisherContext::hDispatchDelegate, SelfType, publishDispatch, this));\n")
    outfile.write("        // Write out parameters to params\n")
    outfile.write("        if (params) {\n")
    for i in range(0, argcount):
        temp = Template("            params->param${itr}_ = param$itr;\n");
        outfile.write(temp.substitute(itr=i+1))
    outfile.write("        }\n")
    outfile.write("    }\n")
    # Immediate version
    outfile.write("    void operator () (")
    for i in range(0, argcount):
        if i > 0: outfile.write(", ")
        outfile.write(Template("Param${itr}Type param$itr").substitute(itr=i+1))
    outfile.write(")\n")
    outfile.write("    {\n")
    outfile.write("        for (hUint i=0, c=subscribers_.size(); i<c; ++i) {\n")
    outfile.write("            subscribers_[i](")
    for i in range(0, argcount):
        if i > 0: outfile.write(", ")
        outfile.write(Template("param$itr").substitute(itr=i+1))
    outfile.write(");\n")
    outfile.write("        }\n")
    outfile.write("    }\n")
    outfile.write("    void connect(const Delegate& proc)\n")
    outfile.write("    {\n")
    outfile.write("        subscribers_.push_back(proc);\n")
    outfile.write("    }\n")
    outfile.write("    void disconnect(const Delegate& proc)\n")
    outfile.write("    {\n")
    outfile.write("        //This doesn't maintain order but I'm not sure that it matters,\n")
    outfile.write("        for (hUint i=0, c=subscribers_.size(); i<c; ++i) {\n")
    outfile.write("            if (subscribers_[i] == proc) {\n")
    outfile.write("                subscribers_[i] = subscribers_[c-1];\n")
    outfile.write("                subscribers_.resize(c-1);\n")
    outfile.write("            }\n")
    outfile.write("        }\n")
    outfile.write("    }\n")
    outfile.write("private:\n")
    outfile.write("    struct ParamStruct\n")
    outfile.write("    {\n")
    for i in range(0, argcount):
        temp = Template("        Param${itr}Type param${itr}_;\n");
        outfile.write(temp.substitute(itr=i+1))
    outfile.write("    };\n")
    outfile.write("    typedef hVector< Delegate > SubscriberArrayType;\n")
    outfile.write("    void publishDispatch(void* data, hUint size)\n")
    outfile.write("    {\n")
    outfile.write("        ParamStruct* params = (ParamStruct*)data;\n")
    outfile.write("        for (hUint i=0, c=subscribers_.size(); i<c; ++i) {\n")
    outfile.write("            subscribers_[i](")
    for i in range(0, argcount):
        if i > 0: outfile.write(", ")
        temp = Template("params->param${itr}_");
        outfile.write(temp.substitute(itr=i+1))
    outfile.write(");\n")
    outfile.write("        }\n")
    outfile.write("    }\n")
    outfile.write("    SubscriberArrayType subscribers_;\n")
    outfile.write("};\n")


def main():
    parser = argparse.ArgumentParser(prog="event_publisher",description='Generates publisher code definitions')
    parser.add_argument('-i','--iterations',  type=int, help='Max template arguments to create publishers with')
    parser.add_argument('-o','--output', help='Name of the output file')
    
    args = parser.parse_args()
    
    path, file = os.path.split(args.output)
    
    outputFile = open(args.output, "w+")
    
    outputTxtHeader(outputFile, file)
    for i in range(0, args.iterations):
        outputPublisherClassDef(outputFile, i);
    
    print args.output, "written"
    
if __name__ == '__main__':
    main()