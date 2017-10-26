/* License and copyright go here*/


// adjectives used in shop names.

// This is in a separate source file as we intend to reuse it for things other than shops at some point.

extern const wchar_t * const shopAdjectives[] {
    L"Amusing",
    L"Bountiful",
    L"Charming",
    L"Dangerous",
    L"Enchanting", // bestows enchantments
    L"Friendly", // offers a complimentary beverage
    L"Generous",
    L"Hearty",
    L"Indispensible",
    L"Juxtoposing",
    L"Kind",
    L"Likely",
    L"Madcap",
    L"Naughty",
    L"Old-Fashioned",
    L"Protective", // bestows proofing
    L"Quaint",
    L"Rustic",
    L"Salty",
    L"Tasteful",
    L"Unimaginative",
    L"Voluptuous",
    L"Wise",
    L"Xenial",
    L"Yellowish",
    L"Zesty",
      //    L"& co.",
      //    L"& family",
    L"Þe Olde", // doesn't actually make gramatical sense, but too good an apportunity to miss...
      // (ref: Old English wrote "Th" with the letter Þ (thorn), often rendered as "Y" in typing.
      // so "Ye Olde foo shop" became a stereotype for an old shop)
      };
// Index to one-past the end of the array.
// 27 letters for the entire alphabet (including "&", which was once considered a letter)
// then 1 extra "&" and a thorn...
// ... then I didn't like the "foo's & co shiny things emporium" format, so the &s had to go
extern const int numShopAdjectives = 27;

