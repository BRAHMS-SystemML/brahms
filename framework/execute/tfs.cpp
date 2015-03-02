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

	$Id:: tfs.cpp 2309 2009-11-06 00:49:32Z benjmitch          $
	$Rev:: 2309                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-06 00:49:32 +0000 (Fri, 06 Nov 2009)       $
________________________________________________________________

*/

// Make sure UINT32 is in scope
#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h"

#ifdef __GLN__
#include <stdlib.h>
#endif

#include <iostream>
using std::cout;
#include <string>
using std::string;
#include <vector>
using std::vector;

////////////////	TRUE/FALSE/STUPID FUNCTION

#define tfs_out cout

void breakprint(string msg)
{
    if (!msg.length()) return;
    for (UINT32 n=0; n<msg.length(); n++)
    {
        if (msg[n] == 32 && n>44)
        {
            string line = msg.substr(0,n);
            tfs_out << "    " + line + "\n";
            breakprint(msg.substr(n+1));
            return;
        }
    }
    tfs_out << "    " + msg + "\n";
}

void tfs(void)
{
    vector<string> tfs;
    tfs.push_back("An elephant's trunk is equivalent to the human nose and upper lip - yet an adult elephant can carry nearly 270 kilograms with it. It can also store about 4 litres of water in it.");
    tfs.push_back("A giraffe has the same number of bones in its neck as a human. They can also clean their ears with their half metre long tongue.");
    tfs.push_back("Oysters can change from one gender to another and back again depending on which is best for mating.");
    tfs.push_back("Camels have three eyelids - these help protect their eyes from the sun and sand in the desert.");
    tfs.push_back("Fish are very good at tasting. They not only have taste buds in their mouths but some species also have them on their skin and sometimes on their fins. Catfish even have taste buds on their whiskers.");
    tfs.push_back("Jellyfish are more than 95% water. They have no heart, bones, or brain, and have no real eyes. Yet the largest jellyfish, the Lion's Mane jellyfish, can reach 8 feet across with 200 feet long tentacles.");
    tfs.push_back("If all the blood vessels in the adult human body were laid end to end they would stretch 160,000 kilometres, that's four times around the Earth.");
    tfs.push_back("Orcas are voluntary breathers. They have to sleep with only half of their brain at one time. The other half remains awake to regulate their breathing.");
    tfs.push_back("The common goldfish is the only animal that can see both infra-red and ultra-violet light but their sight deteriorates quickly with age and UV vision is soon lost.");
    tfs.push_back("The number of beneficial bacteria in the human intestine is equal to the number of cells in the human body.");
    tfs.push_back("Batology is the taxonomic study of blackberries. Batologists classify and categorise the 1000 different species of blackberry.");
    tfs.push_back("Nearly one million earthworms can be found in a single acre of land.");
    tfs.push_back("There are more chickens in the world than people. In the UK alone there are 30.6 million hens laying eggs - that's half a hen for each member of the population. On top of this, 807 million chickens are consumed each year in the UK and at any one time there are 113 million of these strutting their stuff.");
    tfs.push_back("Water is actually a blue colour when there is enough of it. Although water looks clear in a glass, from space the oceans are definitely blue.");
    tfs.push_back("There are 6 kilograms of gold per cubic kilometre of sea water.");
    tfs.push_back("It doesn't matter when badgers mate - the cubs are always born around February. This can happen because the female badger controls when the fertilised egg begins to develop.");
    tfs.push_back("The smallest mammal in the world is probably the pygmy shrew. It weighs just two grams. To stay alive its heart has to beat over 1,200 times a minute and it must eat every few hours, day and night or it will starve to death.");
    tfs.push_back("Some Arctic and Antarctic fish have evolved proteins in their blood which act as antifreeze stopping their blood from freezing solid.");
    tfs.push_back("The sperm whale is the largest predator that has ever lived.");
    tfs.push_back("The blue ringed octopus, found in Southeast Asia, carries such a powerful neurotoxin that just one octopus contains enough to cause paralysis and death of ten adult humans.");
    tfs.push_back("The tongue of a blue whale is the size of an elephant. The entire whale generally weighs the same as about 30 elephants.");
    tfs.push_back("The pirate spider invades other spiders' webs by creeping so slowly that the resident spider does not realise what is going on. The pirate spider can get close enough to bite, injecting a very toxic venom.");
    tfs.push_back("Starfish have no brain - they're just a bag of nerves.");
    tfs.push_back("Size isn't everything when it comes to planets. Seven moons in the solar system are larger than the planet Pluto. These are Jupiter's moons, Ganymede, Callisto, Io and Europa; Saturn's moon, Titan; Neptune's moon, Triton; and our own Moon.");
    tfs.push_back("The Sun loses up to a billion kilograms every second because of the solar wind that blasts out from its surface.");
    tfs.push_back("When is a constellation not a constellation? When it's an asterism. An asterism is a sub-pattern within a constellation and perhaps the most famous asterism is the Plough. The Plough consists of just seven of the 16 named stars of the constellation Ursa Major.");
    tfs.push_back("The Moon is gradually moving away from the Earth and the tides are to blame. Every year, the Moon moves a further 3.82cm from the Earth.");
    tfs.push_back("Jupiter's moon Io has the most spectacular volcanoes in the solar system. Io's low gravity means that its volcanoes can eject material 75 kilometres above the surface, with lighter material reaching hundreds of kilometres.");
    tfs.push_back("Some neutron stars can spin up to 1000 times a second. More earthly objects such as CD-ROMs can spin up to 100 times a second whereas music CDs spin a meagre 10 times a second.");
    tfs.push_back("The first space shuttle was given the name 'Enterprise' because of a campaign by Star Trek fans. The shuttle was originally going to be called 'Constitution'.");
    tfs.push_back("In the 1970s, Pan American World Airways accepted reservations for flights to the Moon in the year 2000. Former president Ronald Reagan was one of the many who signed up.");
    tfs.push_back("When pulsars were discovered in 1968, astronomers designated them 'LGM' or 'Little Green Men' because they were unsure of their actual nature. However, no one seriously believed that these were alien signals.");
    tfs.push_back("The electric chair was invented by a dentist, Alfred Southwick.");
    tfs.push_back("IBM's ASCI white supercomputer, the fastest computer in the world, weighs as much as 17 elephants and can do in one second what a calculator would take 10 million years to do.");
    tfs.push_back("The Wide Field and Planetary Camera on the Hubble Space Telescope, could resolve the fine print on a newspaper one mile away.");
    tfs.push_back("The microwave oven was invented by accident, when Percy Spencer found that his chocolate bar had been melted by an experiment he was running on radar systems. He immediately started experimenting successfully on microwaved popcorn.");
    tfs.push_back("The total length of all the blood vessels in the human body is about 97,000 km. This is over twice the circumference of the Earth's equator.");
    tfs.push_back("In a lifetime we spend the same amount of time eating as we do blinking. We spend about five years eating, and about five years with our eyes shut because we are blinking.");
    tfs.push_back("As we get older the brain loses almost one gram per year because its nerve cells die and cannot be replaced.");
    tfs.push_back("We discard about 10 billion skin flakes every day. Over a year this mounts up to nearly two kilograms.");
    tfs.push_back("A mouse has a larger ratio of brain size to body mass than a human, it used to be believed that this ratio was a sign of intelligence - if this were true mice would be about twice as clever as humans.");
    tfs.push_back("Of all the nerve receptors in the human body, pain nerve endings are the most common.");
    tfs.push_back("The most sensitive parts of the body are the fingers and lips whilst the least sensitive part is the middle of the back.");
    tfs.push_back("A rod in the human eye can detect a single photon of light. This is the amount of light that might reach the eye from a candle, a mile away.");
    tfs.push_back("The life span of a taste bud is ten days but the cells are constantly being renewed, roughly one every ten hours.");
    tfs.push_back("It is estimated that in mid 2001, there were 9,000 people aged 100 and over in the UK.");
    tfs.push_back("A left handed person finds it easier to open a jar than a right handed person. This is because a left handed person can supply a stronger anticlockwise turning force than a right handed person. However a right handed person will find it easier to tighten the jar up afterwards.");
    tfs.push_back("In 1960 Joe W. Kittinger, Jr. jumped from a weather balloon 32 kilometres above the Earth's surface. This is above the ozone layer and well above the height that jet planes fly at. During his freefall he reached the speed of sound.");
    tfs.push_back("The tallest structure in the World - the CN tower in Toronto - is over half a kilometre high.");
    tfs.push_back("Einstein's theory of relativity actually shows that everything is not relative. The speed of light is always the same no matter how fast you are going - away from or towards the beam.");
    tfs.push_back("Research has shown that plants grow better when they are stroked, so a new machine has been invented which strokes seedlings as they are geminating, producing stronger, healthier plants.");
    tfs.push_back("In the last 50 years the temperature of the Antarctic has risen by 2.5 degrees Celsius and may be to blame for icebergs breaking away from the continent. In 1995 a 1000 square kilometre iceberg broke off and started to float north.");
    tfs.push_back("The complexity of the organism has no bearing on the number of chromosomes it has: humans have 23 pairs of chromosomes while a species of fern holds the record for the most, with 630 pairs.");
    tfs.push_back("The silkworm moth (Bombyx mori) is the only completely domesticated insect. They are no longer found in the wild and have been cultivated for so long that they cannot fly.");
    tfs.push_back("Silkworm larvae require a tonne of mulberry leaves to produce a 5kg of silk.");
    tfs.push_back("Every rainbow is unique - each rainbow is formed from light hitting your eye at a very precise angle. Someone standing next to you will see light coming from a slightly different angle than you and therefore see a different rainbow.");
    tfs.push_back("The energy in one hurricane is equal to about 500,000 atomic bombs.");
    tfs.push_back("The largest hailstone ever to hit the Earth fell in 1970. It was 14 centimetres across.");
    tfs.push_back("The Mariana Trench in the Pacific Ocean is so deep that Mount Everest could be submerged in it with its summit still two kilometres below the surface.");
    tfs.push_back("There is about 6 kg of gold per cubic kilometre of sea water. The oceans contain more than one billion cubic kilometres of water which means there is 6 million tonnes of gold in the Earth's oceans.");
    tfs.push_back("Icebergs are not restricted to the Arctic or Antarctic - they can float very far south or north. An Arctic iceberg has been sighted at the same latitude as Florida and an Antarctic iceberg at the same latitude as Rio de Janeiro.");
    tfs.push_back("The wettest place in the world is the island of Kauai, Hawaii. One side of the island receives 11.5 metres of rainfall a year, but the other side receives only 0.25 metres.");
    tfs.push_back("Because of the depth of the Pacific Ocean, pacific tsunami (waves created by earthquakes) can reach speeds of over 750 kilometres per hour. The same velocity as a jet aeroplane.");
    tfs.push_back("In the middle of the Atlantic the two plates, the African Plate and the American plate, are moving apart at about the same speed as your fingernails grow.");
    tfs.push_back("The centre of the Earth's core is thought to exceed 4000 K - similar to the temperature of the surface of the sun. The pressure at the centre of the Earth is estimated to be 4 million atmospheres.");
    tfs.push_back("Ironically there is no gravity at the centre of the Earth because it all cancels out.");
    tfs.push_back("A bolt of lightning contains enough energy to toast 160,000 pieces of bread. Unfortunately the bolt only takes 1/10,000 of a second � so turning the bread over might prove difficult.");
    tfs.push_back("When an impala is alarmed by a predator it can jump 3 metres in air and 11 metres along the ground. This beats both the world high jump and world long jump records in one go.");
    tfs.push_back("One sheep can supply enough wool to make about 14 jumpers.");
    tfs.push_back("Sapphires and rubies are both made of the mineral corundum. They are identical in every way except their colour. Rubies are the red variety while sapphires can be any other colour, blue, green, yellow, orange or even black.");
    tfs.push_back("Most colours have a wavelength, but brown doesn't. It's just a combination of other colours and their wavelengths.");
    tfs.push_back("If 10 kilograms of matter spontaneously turned into energy there would be enough energy to power a 100 Watt light bulb for 300 million years - a harrowing thought for all weight watchers.");
    tfs.push_back("A supernova is the most energetic single event known in the Universe. Material is exploded into space at a speed of about 10,000 kilometres per second and the energy emitted is 10 to the power of 44 Joules. Our galaxy contains about 100,000,000,000 stars and all these stars would have to shine for six months to produce this much energy.");
    tfs.push_back("The orbit of the Moon about the Earth would fit easily inside the Sun.");
    tfs.push_back("Gold leaf is pure gold, but you can cover large areas with it very cheaply because it is very thin. Gold leaf is less than 0.00008 millimetres thick - which is only about 300 atoms thick.");
    tfs.push_back("Paper money is made from cotton rather than from wood pulp which means it doesn't disintegrate as easily as ordinary paper if you leave it in your laundry.");
    tfs.push_back("The mass of the Earth increases every year because of the 3,000 tonnes of meteorite debris that hits its surface from space.");
    tfs.push_back("The Rosetta Stone has recently been proved to be a fake. It was fabricated by the French soldiers, trapped in Egypt, as a diversionary tactic. The true meaning of the Hieroglyphics found on the walls of tombs and ancient artefact has only just come to light. They were in fact nothing more than graffiti; which correctly translate to things such as 'Cairo skins rule', 'Luxor United are magic', and 'Some time in the future you will receive vicious multiple blows to your copulating cranium, and appendages of the lower limbs will be used to inflict the damage'.");
    tfs.push_back("The electrical impulses that run through the brains of small mammals, such as mice, exactly match the frequencies of the electrical components in most pocket organisers. If an organiser is switched on and placed next to head of, say, a pet mouse, the interference causes words to form on the screen which match those of the thought processes of the mouse. The most common words seen are 'hungry' and 'tired', but sometimes more complex messages are seen, such as 'stop poking me in the head with that thing!'");
    tfs.push_back("The scientists at CERN, in Geneva, stopped doing any experiments in 1998. Since then they have been using the giant accelerators to play 'particle skittles'. Single carbon atoms are etched in a triangular pattern onto sheets of gold leaf, which are then bombarded with pulses of high energy electrons. The winner is the one who blasts away the greatest number of atoms. This originally started out as as a minaturisation project for a PhD student, and now has the whole scientific community hooked.");
    tfs.push_back("The technical term for those people that reach underneath the dividing partitions in public toilets and try to grab hold of your feet is 'panorixians'. Nearly three quarters of these people were found to have suffered a severe traumatic episode involving a pet hamster between the ages of 2 and 4.");
    tfs.push_back("If satellite pictures of Spain and France are taken, at night, and then the two images superimposed, the resulting picture has a very clear pattern of Greek letters that translate, roughly, as 'Beware the turnips. They are not what they seem'.");
    tfs.push_back("The Go-Go frog of South America is the only known animal that regularly eats parts of its own body. When food is in short supply it is quite common for this small amphibian to eat three of its legs in a day. The legs usually grown back again within 4 or 5 hours. One of these frogs has even been observed swallowing its own head, which unfortunately resulted in its death.");
    tfs.push_back("Contrary to popular belief, bathing in asses milk is not good for the complexion. In fact it can proove to be fatal. After about 45 minutes soaking in the milk the skin will begin to dissolve. After 90 minutes the results are similar to full thickness 2nd degree burns.");
    tfs.push_back("The colours that we see today are not the same as those seen by our ancestors. There has been a gradual shift in colour perception due to changes in lifestyle over the past 50 years or so. In the 1950's people would have described what we now call 'light blue' as 'mauve', and 'yellow' as 'turquoise'. All current computer monitors are built to allow for this colour shift, but unfortunately TV manufacturers have deemed it too expensive. Already people are begining to notice subtle differences between the colours displayed on these two devices.");
    tfs.push_back("Following on from the recent successes utilising the combined surplus processor power of 'idle' computers on the internet, electricity suppliers have recently announced a massive wind turbine project. The power of the air blown out from the backs of computers can be harnessed, using a small blade attached to an electric generator (similar to those seen in wind farms). These generators can be plugged into a device that attaches to the mains supply in order to feed back electricity into the system. It has been estimated that if all the computers in Britain were used in this way it would generate enough energy to power a small country in Africa.");
    tfs.push_back("Bread has recently been discovered to be highly radioactive. The reason this was not discovered earlier is due to the unusual nature of the decay process. Normal radioactive decay ejects alpha, beta or gamma particles. As bread decays it emits both alpha and beta radiation which, due to the texture of the bread, interact to form a new 'delta' radiation. This new type of radiation is very difficult to detect by normal means.");
    tfs.push_back("Every year 14 people are arrested for 'possession of a deformed halibut'. This is one of the world's most widely ranging, albeit obscure, laws - valid in 95% of civilised countries. The offence carries a maximum sentence of 4 days imprisonment.");
    tfs.push_back("The Great Snail Sucking Contest in Llanbedr, Wales, attracts more visiting tourists during it's three day duration than seven. (Usually at least eight or nine turn up). The object of the contest is to determine who can suck the greatest number of snails from their shells in a 30 minute period. The winner receives a gold-plated turnip from the mayor, which he or she is obliged to wear around the neck until the next contest the following year.");
    tfs.push_back("A new strain of bacteria, called hydroglopitica treviticus, has been recently discovered. It has the ability to swim with incredible speed through urine. It spreads by swimming from a toilet bowl up a stream of urine and into a persons body, where it multiplies and is then passed on to other toilet bowls.");
    tfs.push_back("It has been estimated that if satellites are put into orbit at their current rate then by the year 2078 the amount of light reaching the surface of the earth will be reduced by 50%.");
    tfs.push_back("If every person on the planet breathed in at exactly the same time it would cause a pressure drop that would raise the sea level by 4.2 cm.");
    tfs.push_back("Green is the heaviest colour. It is more than 20% heavier than the next heaviest colour, which is blue. The lightest colour so far discovered is a particular shade of pink found on a lampshade in the B&Q store in Eastbourne.");
    tfs.push_back("If you laid all the bananas that have ever been eaten since 1953 end to end they would stretch from the Post Office in the centre of Luton all the way to table number four in the back room of the Horse and Pilchard public house in Little Husbrock on the Marsh.");
    tfs.push_back("The gestation period for gerbils can be increased up to 24 months by the continuous playing of Perry Como albums along with a diet of dry roast peanuts. Any attempt to go beyond 24 months usually results in spontaneous combustion of the animal.");
    tfs.push_back("Turkeys are the most prolific carnivores of all birds. A horse, if locked in a shed with a turkey, will be stripped to the bone in less than 15 minutes.");
    tfs.push_back("The world record for toad balancing currently stands at 28. The recent attempt to balance 35 toads was discredited after it was discovered that several of the toads had been stapled together.");
    tfs.push_back("Statistics show that people with green eyes are more than twice as likely to be run over by a number 26 bus than people with blue eyes.");
    tfs.push_back("The average public toilet seat is home to 48 different species of bacteria. This compares to 5 species found on the average household toilet seat.");
    tfs.push_back("A survey conducted in 1976, in Wales, showed that 43% of the male population had more than 4 pairs of shoes. 64% of shoes were black and 32% were brown.");
    tfs.push_back("Pineapples are the most dangerous fruit. If mixed in the correct proportions with brandy and baking soda they combine to form a chemical with more explosive power than TNT.");
    tfs.push_back("The oldest lighthouse in the world is not known. The first definite and documented lighthouse in the world was the Pharos of Alexandria, built in about 200 BC, although beacons were certainly used before that time. The oldest working lighthouse in the world is at La Coru�a in NW Spain, near the town of Ferol. A lighthouse has been on this site since the time of the Roman emperor Trajan.");
    tfs.push_back("The oldest lighthouse in the UK still stands in the grounds of Dover Castle. The Roman Emperor Caligula ordered the tower to be built there in AD 90.");
    tfs.push_back("The world's first stone lighthouse tower at sea was the Smeaton Eddystone lighthouse, built in 1756-9. Smeaton is today known as 'The Father of Civil Engineering'. He invented many new engineering designs for his lighthouse, including the dovetailing of rocks, marine cements and special cranes to lift rocks out of a boat and onto the reef. When his lighthouse was finished, it was lit with a mere twenty-four candles. Today, the power of lighthouse lights could be equivalent to as many as several million candles.");
    tfs.push_back("On one occasion, lighthouse keepers were forced to eat candles to survive when they were marooned on a lighthouse in bad weather. The candles were not wax candles, like we use today, but made from oil-based material that was digestible.");
    tfs.push_back("Many early lighthouses were simply lamps held in high windows by monks and hermits. Later, coal fires were used on the top of open towers, but they made so much smoke that they were frequently invisible from the sea.");
    tfs.push_back("Soon after electricity was invented, the first practical application of electricity was to power the lights in lighthouses. Michael Faraday himself was frequently to be found visiting lighthouse such as South Foreland where the first experiments with electricity took place. The first electric lights were giant sparks made by passing great voltages across two carbon rods. Until recently, lights were created by lightbulbs similar to those we use at home, but as large as a football. Today, technology has enabled them to be made smaller, but just as bright.");
    tfs.push_back("The magnification of light from the lamps in lighthouse takes place through giant arrangements of curved prisms and lenses which weigh several tons and which float in baths of liquid mercury. Despite their great weight, they will begin to rotate with a gentle push from one finger.");
    tfs.push_back("Mercury vapour is a very poisonous substance, the symptoms of mercury poisoning being madness. It has long been thought that breathing in mercury vapour over a period of years was the reason why some lighthouse keepers went mad. The theory is unproven, however. The vast majority of lighthouse keepers who spent the whole of their working lives in close proximity to these very large masses of mercury remained as normal as you and me.");
    tfs.push_back("When Marconi was experimenting with his first radio transmissions, he also chose to transmit radio messages from South Foreland lighthouse to the South Goodwin lightship.");
    tfs.push_back("The rocks of the Eddystone are always above high water. However, at the Bell Rock lighthouse in Scotland, Robert Stevenson built a lighthouse on a rock that was sometimes beneath the level of high tide.");
    tfs.push_back("Probably the oldest lighthouse keeper was Henry Hall, a keeper on the famous Eddystone lighthouse who was 94. He met a remarkable death on duty. The lighthouse caught fire and, while he tried to put out the fire, he swallowed nearly half a pound of molten lead. He died from lead poisoning about two weeks later.");
    tfs.push_back("There have been many strange disappearances of lighthouse keepers. Perhaps the strangest was at the Flannan Isle lighthouse in 1900 where the three lighthouse keepers disappeared without trace.");
    tfs.push_back("Lighthouse keepers used to catch fish by flying a kite from the balcony of their lighthouses.");
    tfs.push_back("Off the West Coast of England, there are several lighthouses which are more than 45 metres (150 feet) tall. In violent storms, the sea sometimes washes right over the tower, breaking panes of glass in the lantern which were 12.5 mm (half an inch) thick. So much seawater was entering the lantern that the keepers had to tie themselves onto the stair rails to prevent themselves from being washed down the stairs.");
    tfs.push_back("The most unlucky lighthouse builder was Henry Winstanley, who thought he had built the world's strongest lighthouse. He was so confident that the said that he wanted to be inside his lighthouse during the biggest storm ever. His wish came true but his lighthouse did not survive the storm and he was washed away to his death.");
    tfs.push_back("It is said that a keeper of the Longships lighthouse was once kidnapped by Cornish wreckers, but they forgot that his little daughter was still in the lighthouse. Standing on a pile of books, including the family Bible, she was still able to light the oil lamps and keep the lighthouse going until her father was released.");
    tfs.push_back("During World War I, men were employed as temporary keepers who were not fit enough to be soldiers. However, one keeper had to be replaced. The reason was because his wooden leg kept falling off as he tried to climb the stairs.");
    tfs.push_back("Early fog signals involved the keepers detonating explosive charges every few minutes from the gallery. The sound of the explosion was the warning for ships to avoid the rocks.");
    tfs.push_back("A light known as the Lanterna was built at Genoa in 1543, replacing a medieval tower. It still dominates the harbour as one of the tallest brick or masonry lighthouses in the world at 75 m height. The Ile de Vierge off the coast of Brittany, France, is 83 m tall and is the current contender for the world's tallest traditionally built lighthouse. However, a steel light tower at Yamashita Park, Yokohama, Japan is 106 m tall.");
    tfs.push_back("The tallest lighthouse in Britain is the Skerryvore lighthouse off the west coast of Scotland. It was built in 1844 by Robert Stevenson and is 49 m high and built of 4,300 tons of granite. In France, the light at Cordouan is 57 m above sea level, whilst the tower at Les Heaux de Brehat is about the same height at Skerryvore, as also is the present Fastnet tower off the south-west coast of Ireland. The tallest lighthouse built of cast iron was one by Alexander Gordon at Gibb's Hill, Bermuda which stands at 40.8 m high.");
    tfs.push_back("Antonio Columbo, uncle of Christopher Columbus, was a lighthouse keeper of the famous Lanterna of Genoa in 1449. Perhaps his uncle's connection with seafaring, a breakaway from the family's traditional occupation of weaving, led to Christopher's interest in going to sea.");
    tfs.push_back("The lighthouse with the most doors was an old Roman lighthouse known as the Tour d'Ordre and built at Boulogne in France - it had 96!");
    tfs.push_back("It is said that in the 14th century, an Abbott placed a bell on a dangerous rock known as the Inchcape rock off Arbroath in Scotland. Attached to wooden buoy, it was continuously rung by the sea, warning ships of the danger. A local pirate removed the bell, but himself was drowned when his own ship struck the rocks. This reef is now well known as the Bell Rock and the story was turned into a popular poem in 1815 by Robert Southey.");
    tfs.push_back("The Sambro lighthouse at the entrance to Halifax Harbour in Canada is thought to be the oldest still in use in north America. Soon after it was first lit, it became the focal point of a scandal. After a Royal Navy warship was wrecked nearby, naval captains reported that they had to fire cannons at the lighthouse to persuade the keeper to show his light.");
    tfs.push_back("The lighthouse station called Arctowski is probably the most southerly lighthouse in the world. Built at the Polish research station in Antarctica named after Henryk Arctowski, the famous 19th century Polish geographer and Antarctic explorer. Situated on King George Island in the South Shetlands group, its geographical position is 62o10'S, 58o28'W.");
    tfs.push_back("In 1895 a new species of wren was discovered unique to Stephen's Island, New Zealand. The Stephen's Island wren was identified only from dead specimens: the last had been killed by the lighthouse keeper's cat.");
    tfs.push_back("Over the years, some remarkable ideas have been proposed for application in lighthouses, besides the sensible ones that were adopted. To distinguish the Scilly lighthouse from others and thus to reduce the frequency of wrecks in the vicinity, William Whiston proposed 'that a Ball of Light or Fire be thrown up from St. Mary's, the principal of the English Isles of Scilly every Mid-night and three Times more every Night'.");
    tfs.push_back("The following is the transcript of the actual radio conversation of a US naval ship with Canadian authorities off the coast of Newfoundland:\n\nCanadians: Please divert your course 15 degrees the South to avoid a collision.\nAmericans: Recommend you divert your course 15 degrees the North to avoid a collision.\nCanadians: Negative. You will have to divert your course 15 degrees to the South to avoid a collision.\nAmericans: This is the Captain of a US Navy ship. I say again, divert YOUR course.\nCanadians: No. I say again, you divert YOUR course.\nAmericans: This is the aircraft carrier USS lincoln, the second largest ship in the united states' atlantic fleet. We are accompanied by three destroyers, three cruisers and numerous support vessels. I demand that you change your course 15 degrees north, i say again, that's one five degrees north, or counter-measures will be undertaken to ensure the safety of this ship.\nCanadians: This is a lighthouse, over...");
    tfs.push_back("'Without craftsmanship, inspiration is a mere reed shaken in the wind' - Johannes Brahms");

    srand(time(NULL));
    UINT32 i = rand() % tfs.size();

    if (i < tfs.size())
    {
        tfs_out << "True, false, or stupid?\n\n";
        breakprint(tfs[i]);
    }
}
