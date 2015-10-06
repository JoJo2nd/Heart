/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.

    Original found at http://www.gamedev.net/topic/648016-replacement-for-id3dxconstanttable/
    and based on wine-1.6\dlls\d3dx9_36\shader.c. Saved me reverse engineering it.
*********************************************************************/
#include <vector>
#include <string>
#include <cstdint>

#pragma once

/* -- https://github.com/wine-compholio/wine-staging/tree/master/patches/d3dx9_36-GetShaderSemantics
static const DWORD* skip_instruction(const DWORD *byte_code, UINT shader_model)
{
    TRACE("Shader model %u\n", shader_model);

    /* Handle all special instructions whose arguments may contain D3DSIO_DCL *
    if ((*byte_code & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT)
    {
        byte_code += 1 + ((*byte_code & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT);
    }
    else if (shader_model >= 2)
    {
        byte_code += 1 + ((*byte_code & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT);
    }
    else if ((*byte_code & D3DSI_OPCODE_MASK) == D3DSIO_DEF)
    {
        byte_code += 1 + 5;
    }
    else
    {
        /* Handle remaining safe instructions *
        while (*++byte_code & (1 << 31));
    }

    return byte_code;
}

static UINT get_shader_semantics(const DWORD *byte_code, D3DXSEMANTIC *semantics, DWORD type)
{
    const DWORD *ptr = byte_code;
    UINT shader_model = (*ptr >> 8) & 0xff;
    UINT i = 0;

    TRACE("Shader version: %#x\n", *ptr);
    ptr++;

    while (*ptr != D3DSIO_END)
    {
        if (*ptr & (1 << 31))
        {
            FIXME("Opcode expected\n");
            return 0;
        }
        else if ((*ptr & D3DSI_OPCODE_MASK) == D3DSIO_DCL)
        {
            DWORD param1 = *++ptr;
            DWORD param2 = *++ptr;
            DWORD usage = param1 & 0x1f;
            DWORD usage_index = (param1 >> 16) & 0xf;
            DWORD reg_type = (((param2 >> 11) & 0x3) << 3) | ((param2 >> 28) & 0x7);

            TRACE("D3DSIO_DCL param1: %#x, param2: %#x, usage: %u, usage_index: %u, reg_type: %u\n",
                   param1, param2, usage, usage_index, reg_type);

            if (reg_type == type)
            {
                if (semantics)
                {
                    semantics[i].Usage = usage;
                    semantics[i].UsageIndex = usage_index;
                }
                i++;
            }

            ptr++;
        }
        else
        {
            ptr = skip_instruction(ptr, shader_model);
        }
    }

    return i;
}
*/

enum EREGISTER_SET
{
  RS_BOOL,
  RS_INT4,
  RS_FLOAT4,
  RS_SAMPLER
};

struct ConstantDesc
{
  std::string Name;
  EREGISTER_SET RegisterSet;
  int RegisterIndex;
  int RegisterCount;
  int Rows;
  int Columns;
  int Elements;
  int StructMembers;
  size_t Bytes;
};

class ConstantTable
{
public:
  bool Create(const void* data);

  size_t GetConstantCount() const { return m_constants.size(); }
  const std::string& GetCreator() const { return m_creator; } 

  const ConstantDesc* GetConstantByIndex(size_t i) const { return &m_constants[i]; }
  const ConstantDesc* GetConstantByName(const std::string& name) const;

private:
  std::vector<ConstantDesc> m_constants;
  std::string m_creator;
};

// Structs
struct CTHeader
{
  uint32_t Size;
  uint32_t Creator;
  uint32_t Version;
  uint32_t Constants;
  uint32_t ConstantInfo;
  uint32_t Flags;
  uint32_t Target;
};

struct CTInfo
{
  uint32_t Name;
  uint16_t RegisterSet;
  uint16_t RegisterIndex;
  uint16_t RegisterCount;
  uint16_t Reserved;
  uint32_t TypeInfo;
  uint32_t DefaultValue;
};

struct CTType
{
  uint16_t Class;
  uint16_t Type;
  uint16_t Rows;
  uint16_t Columns;
  uint16_t Elements;
  uint16_t StructMembers;
  uint32_t StructMemberInfo;
};

// Shader instruction opcodes
const uint32_t SIO_COMMENT = 0x0000FFFE;
const uint32_t SIO_END = 0x0000FFFF;
const uint32_t SI_OPCODE_MASK = 0x0000FFFF;
const uint32_t SI_COMMENTSIZE_MASK = 0x7FFF0000;
const uint32_t CTAB_CONSTANT = 0x42415443;

// Member functions
bool ConstantTable::Create(const void* data)
{
  const uint32_t* ptr = static_cast<const uint32_t*>(data);
  while(*++ptr != SIO_END)
  {
    if((*ptr & SI_OPCODE_MASK) == SIO_COMMENT)
    {
      // Check for CTAB comment
      uint32_t comment_size = (*ptr & SI_COMMENTSIZE_MASK) >> 16;
      if(*(ptr+1) != CTAB_CONSTANT)
      {
        ptr += comment_size;
        continue;
      }

      // Read header
      const char* ctab = reinterpret_cast<const char*>(ptr+2);
      size_t ctab_size = (comment_size-1)*4;

      const CTHeader* header = reinterpret_cast<const CTHeader*>(ctab);
      if(ctab_size < sizeof(*header) || header->Size != sizeof(*header))
        return false;
      m_creator = ctab + header->Creator;

      // Read constants
      m_constants.reserve(header->Constants);
      const CTInfo* info = reinterpret_cast<const CTInfo*>(ctab + header->ConstantInfo);
      for(uint32_t i = 0; i < header->Constants; ++i)
      {
        const CTType* type = reinterpret_cast<const CTType*>(ctab + info[i].TypeInfo);

        // Fill struct
        ConstantDesc desc;
        desc.Name = ctab + info[i].Name;
        desc.RegisterSet = static_cast<EREGISTER_SET>(info[i].RegisterSet);
        desc.RegisterIndex = info[i].RegisterIndex;
        desc.RegisterCount = info[i].RegisterCount;
        desc.Rows = type->Rows;
        desc.Columns = type->Columns;
        desc.Elements = type->Elements;
        desc.StructMembers = type->StructMembers;
        desc.Bytes = 4 * desc.Elements * desc.Rows * desc.Columns;
        m_constants.push_back(desc);
      }
      return true;
    }
  }
  return false;
}

const ConstantDesc* ConstantTable::GetConstantByName(const std::string& name) const
{
  std::vector<ConstantDesc>::const_iterator it;
  for(it = m_constants.begin(); it != m_constants.end(); ++it)
  {
    if(it->Name == name)
      return &(*it);
  }
  return NULL;
}
