// #include "industrial-element.h"

// IndustrialElement::IndustrialElement(IndustrialElementType elementType)
// {
//     if (elementType == IndustrialElementType::PLC)
//     {
//         m_industrialApplication = CreateObject<PlcApplication>();
//     }
//     else if (elementType == IndustrialElementType::SCADA)
//     {
//         m_industrialApplication = CreateObject<ScadaApplication>();
//     }
//     m_type = elementType;
// }

// Address
// IndustrialElement::GetAddress()
// {
//     return m_address;
// }

// void
// IndustrialElement::SetAddress(Address addr)
// {
//     m_address = addr;
// }

// IndustrialElementType
// IndustrialElement::GetType()
// {
//     return m_type;
// }

// void
// IndustrialElement::ConnectTo(IndustrialElement element)
// {
//     // SCADA to SCADA or PLC to PLC connections are not allowed
//     if (element.GetType() == m_type)
//     {
//         throw std::runtime_error("Cannot connect elements of the same type");
//     }

//     if (m_type == IndustrialElementType::SCADA)
//     {
//         m_industrialApplication -> SetRemote(element.GetAddress());
//     }
// }
