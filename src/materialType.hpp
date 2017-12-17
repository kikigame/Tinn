/* License and copyright go here*/

// types of material

#ifndef MATERIALTYPE_HPP
#define MATERIALTYPE_HPP


enum class materialType {
  clothy, fleshy, glassy, leathery, liquid, metallic, papery, stony, veggy, waxy, woody, END
};

const long unsigned int materialTypeSize = static_cast<unsigned long>(materialType::END);


#endif // ndef MATERIALTYPE_HPP
