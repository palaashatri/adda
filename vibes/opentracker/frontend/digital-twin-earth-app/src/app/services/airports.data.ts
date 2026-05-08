/**
 * Curated subset of major international airports.
 * Source: OurAirports public-domain dataset (https://ourairports.com/data/).
 * Field shape mirrors the OurAirports CSV: iata, icao, name, lat, lon, size.
 */
export interface Airport {
  iata: string;
  icao: string;
  name: string;
  city: string;
  country: string;
  lat: number;
  lon: number;
  size: 'large' | 'medium';
}

export const MAJOR_AIRPORTS: Airport[] = [
  { iata: 'ATL', icao: 'KATL', name: 'Hartsfield–Jackson Atlanta',     city: 'Atlanta',       country: 'US', lat: 33.6407,  lon: -84.4277,  size: 'large' },
  { iata: 'PEK', icao: 'ZBAA', name: 'Beijing Capital',                 city: 'Beijing',       country: 'CN', lat: 40.0801,  lon: 116.5846,  size: 'large' },
  { iata: 'LAX', icao: 'KLAX', name: 'Los Angeles Intl',                city: 'Los Angeles',   country: 'US', lat: 33.9416,  lon: -118.4085, size: 'large' },
  { iata: 'DXB', icao: 'OMDB', name: 'Dubai Intl',                      city: 'Dubai',         country: 'AE', lat: 25.2532,  lon: 55.3657,   size: 'large' },
  { iata: 'HND', icao: 'RJTT', name: 'Tokyo Haneda',                    city: 'Tokyo',         country: 'JP', lat: 35.5523,  lon: 139.7798,  size: 'large' },
  { iata: 'ORD', icao: 'KORD', name: 'O’Hare Intl',                city: 'Chicago',       country: 'US', lat: 41.9786,  lon: -87.9048,  size: 'large' },
  { iata: 'LHR', icao: 'EGLL', name: 'London Heathrow',                 city: 'London',        country: 'GB', lat: 51.4700,  lon: -0.4543,   size: 'large' },
  { iata: 'PVG', icao: 'ZSPD', name: 'Shanghai Pudong',                 city: 'Shanghai',      country: 'CN', lat: 31.1443,  lon: 121.8083,  size: 'large' },
  { iata: 'CDG', icao: 'LFPG', name: 'Paris Charles de Gaulle',         city: 'Paris',         country: 'FR', lat: 49.0097,  lon: 2.5479,    size: 'large' },
  { iata: 'DFW', icao: 'KDFW', name: 'Dallas/Fort Worth',               city: 'Dallas',        country: 'US', lat: 32.8998,  lon: -97.0403,  size: 'large' },
  { iata: 'AMS', icao: 'EHAM', name: 'Amsterdam Schiphol',              city: 'Amsterdam',     country: 'NL', lat: 52.3105,  lon: 4.7683,    size: 'large' },
  { iata: 'FRA', icao: 'EDDF', name: 'Frankfurt am Main',               city: 'Frankfurt',     country: 'DE', lat: 50.0379,  lon: 8.5622,    size: 'large' },
  { iata: 'IST', icao: 'LTFM', name: 'Istanbul',                        city: 'Istanbul',      country: 'TR', lat: 41.2753,  lon: 28.7519,   size: 'large' },
  { iata: 'CAN', icao: 'ZGGG', name: 'Guangzhou Baiyun',                city: 'Guangzhou',     country: 'CN', lat: 23.3924,  lon: 113.2988,  size: 'large' },
  { iata: 'SIN', icao: 'WSSS', name: 'Singapore Changi',                city: 'Singapore',     country: 'SG', lat: 1.3644,   lon: 103.9915,  size: 'large' },
  { iata: 'DEL', icao: 'VIDP', name: 'Indira Gandhi Intl',              city: 'Delhi',         country: 'IN', lat: 28.5562,  lon: 77.1000,   size: 'large' },
  { iata: 'BOM', icao: 'VABB', name: 'Chhatrapati Shivaji Maharaj',     city: 'Mumbai',        country: 'IN', lat: 19.0896,  lon: 72.8656,   size: 'large' },
  { iata: 'ICN', icao: 'RKSI', name: 'Incheon Intl',                    city: 'Seoul',         country: 'KR', lat: 37.4691,  lon: 126.4505,  size: 'large' },
  { iata: 'JFK', icao: 'KJFK', name: 'John F. Kennedy Intl',            city: 'New York',      country: 'US', lat: 40.6413,  lon: -73.7781,  size: 'large' },
  { iata: 'DEN', icao: 'KDEN', name: 'Denver Intl',                     city: 'Denver',        country: 'US', lat: 39.8561,  lon: -104.6737, size: 'large' },
  { iata: 'SFO', icao: 'KSFO', name: 'San Francisco Intl',              city: 'San Francisco', country: 'US', lat: 37.6213,  lon: -122.3790, size: 'large' },
  { iata: 'SEA', icao: 'KSEA', name: 'Seattle–Tacoma',             city: 'Seattle',       country: 'US', lat: 47.4502,  lon: -122.3088, size: 'large' },
  { iata: 'MIA', icao: 'KMIA', name: 'Miami Intl',                      city: 'Miami',         country: 'US', lat: 25.7959,  lon: -80.2870,  size: 'large' },
  { iata: 'YYZ', icao: 'CYYZ', name: 'Toronto Pearson',                 city: 'Toronto',       country: 'CA', lat: 43.6777,  lon: -79.6248,  size: 'large' },
  { iata: 'GRU', icao: 'SBGR', name: 'São Paulo Guarulhos',         city: 'São Paulo',         country: 'BR', lat: -23.4356, lon: -46.4731,  size: 'large' },
  { iata: 'MEX', icao: 'MMMX', name: 'Mexico City',                     city: 'Mexico City',   country: 'MX', lat: 19.4361,  lon: -99.0719,  size: 'large' },
  { iata: 'EZE', icao: 'SAEZ', name: 'Buenos Aires Ezeiza',             city: 'Buenos Aires',  country: 'AR', lat: -34.8222, lon: -58.5358,  size: 'large' },
  { iata: 'JNB', icao: 'FAOR', name: 'O. R. Tambo Intl',                city: 'Johannesburg',  country: 'ZA', lat: -26.1392, lon: 28.2460,   size: 'large' },
  { iata: 'CAI', icao: 'HECA', name: 'Cairo Intl',                      city: 'Cairo',         country: 'EG', lat: 30.1219,  lon: 31.4056,   size: 'large' },
  { iata: 'SYD', icao: 'YSSY', name: 'Sydney Kingsford Smith',          city: 'Sydney',        country: 'AU', lat: -33.9399, lon: 151.1753,  size: 'large' },
  { iata: 'AKL', icao: 'NZAA', name: 'Auckland',                        city: 'Auckland',      country: 'NZ', lat: -37.0082, lon: 174.7917,  size: 'large' },
  { iata: 'MAD', icao: 'LEMD', name: 'Adolfo Suárez Madrid–Barajas', city: 'Madrid', country: 'ES', lat: 40.4936,  lon: -3.5668,   size: 'large' },
  { iata: 'BCN', icao: 'LEBL', name: 'Barcelona–El Prat',          city: 'Barcelona',     country: 'ES', lat: 41.2974,  lon: 2.0833,    size: 'large' },
  { iata: 'FCO', icao: 'LIRF', name: 'Rome Fiumicino',                  city: 'Rome',          country: 'IT', lat: 41.8003,  lon: 12.2389,   size: 'large' },
  { iata: 'MUC', icao: 'EDDM', name: 'Munich',                          city: 'Munich',        country: 'DE', lat: 48.3538,  lon: 11.7861,   size: 'large' },
  { iata: 'ZRH', icao: 'LSZH', name: 'Zürich',                     city: 'Zurich',        country: 'CH', lat: 47.4647,  lon: 8.5492,    size: 'large' },
  { iata: 'ARN', icao: 'ESSA', name: 'Stockholm Arlanda',               city: 'Stockholm',     country: 'SE', lat: 59.6519,  lon: 17.9186,   size: 'large' },
  { iata: 'SVO', icao: 'UUEE', name: 'Sheremetyevo',                    city: 'Moscow',        country: 'RU', lat: 55.9726,  lon: 37.4146,   size: 'large' },
  { iata: 'BKK', icao: 'VTBS', name: 'Suvarnabhumi',                    city: 'Bangkok',       country: 'TH', lat: 13.6900,  lon: 100.7501,  size: 'large' },
  { iata: 'KUL', icao: 'WMKK', name: 'Kuala Lumpur Intl',               city: 'Kuala Lumpur',  country: 'MY', lat: 2.7456,   lon: 101.7099,  size: 'large' },
  { iata: 'CGK', icao: 'WIII', name: 'Soekarno–Hatta',             city: 'Jakarta',       country: 'ID', lat: -6.1256,  lon: 106.6559,  size: 'large' },
  { iata: 'MNL', icao: 'RPLL', name: 'Ninoy Aquino Intl',               city: 'Manila',        country: 'PH', lat: 14.5086,  lon: 121.0194,  size: 'large' },
  { iata: 'HKG', icao: 'VHHH', name: 'Hong Kong Intl',                  city: 'Hong Kong',     country: 'HK', lat: 22.3080,  lon: 113.9185,  size: 'large' },
  { iata: 'TPE', icao: 'RCTP', name: 'Taoyuan Intl',                    city: 'Taipei',        country: 'TW', lat: 25.0797,  lon: 121.2342,  size: 'large' },
  { iata: 'BOS', icao: 'KBOS', name: 'Boston Logan',                    city: 'Boston',        country: 'US', lat: 42.3656,  lon: -71.0096,  size: 'large' },
  { iata: 'PHX', icao: 'KPHX', name: 'Phoenix Sky Harbor',              city: 'Phoenix',       country: 'US', lat: 33.4342,  lon: -112.0116, size: 'large' },
  { iata: 'LAS', icao: 'KLAS', name: 'Harry Reid Intl',                 city: 'Las Vegas',     country: 'US', lat: 36.0840,  lon: -115.1537, size: 'large' },
  { iata: 'IAH', icao: 'KIAH', name: 'George Bush Intercontinental',    city: 'Houston',       country: 'US', lat: 29.9902,  lon: -95.3368,  size: 'large' },
  { iata: 'YVR', icao: 'CYVR', name: 'Vancouver Intl',                  city: 'Vancouver',     country: 'CA', lat: 49.1939,  lon: -123.1844, size: 'large' },
  { iata: 'DOH', icao: 'OTHH', name: 'Hamad Intl',                      city: 'Doha',          country: 'QA', lat: 25.2731,  lon: 51.6080,   size: 'large' },
  { iata: 'AUH', icao: 'OMAA', name: 'Abu Dhabi Intl',                  city: 'Abu Dhabi',     country: 'AE', lat: 24.4330,  lon: 54.6511,   size: 'large' },
  { iata: 'JED', icao: 'OEJN', name: 'King Abdulaziz Intl',             city: 'Jeddah',        country: 'SA', lat: 21.6796,  lon: 39.1565,   size: 'large' },
  { iata: 'TLV', icao: 'LLBG', name: 'Ben Gurion',                      city: 'Tel Aviv',      country: 'IL', lat: 32.0114,  lon: 34.8867,   size: 'large' },
  { iata: 'ADD', icao: 'HAAB', name: 'Addis Ababa Bole',                city: 'Addis Ababa',   country: 'ET', lat: 8.9779,   lon: 38.7993,   size: 'large' },
  { iata: 'NBO', icao: 'HKJK', name: 'Jomo Kenyatta Intl',              city: 'Nairobi',       country: 'KE', lat: -1.3192,  lon: 36.9278,   size: 'large' },
  { iata: 'LOS', icao: 'DNMM', name: 'Murtala Muhammed',                city: 'Lagos',         country: 'NG', lat: 6.5774,   lon: 3.3211,    size: 'large' },
  { iata: 'CMN', icao: 'GMMN', name: 'Mohammed V',                      city: 'Casablanca',    country: 'MA', lat: 33.3675,  lon: -7.5897,   size: 'large' },
  { iata: 'GIG', icao: 'SBGL', name: 'Rio de Janeiro Galeão',      city: 'Rio de Janeiro',country: 'BR', lat: -22.8120, lon: -43.2506,  size: 'large' },
  { iata: 'BOG', icao: 'SKBO', name: 'El Dorado Intl',                  city: 'Bogotá',   country: 'CO', lat: 4.7016,   lon: -74.1469,  size: 'large' },
  { iata: 'SCL', icao: 'SCEL', name: 'Arturo Merino Benítez',      city: 'Santiago',      country: 'CL', lat: -33.3898, lon: -70.7944,  size: 'large' },
  { iata: 'LIM', icao: 'SPJC', name: 'Jorge Chávez Intl',          city: 'Lima',          country: 'PE', lat: -12.0219, lon: -77.1143,  size: 'large' },
  { iata: 'MEL', icao: 'YMML', name: 'Melbourne Tullamarine',           city: 'Melbourne',     country: 'AU', lat: -37.6690, lon: 144.8410,  size: 'large' },
  { iata: 'PER', icao: 'YPPH', name: 'Perth',                           city: 'Perth',         country: 'AU', lat: -31.9403, lon: 115.9669,  size: 'large' },
  { iata: 'KEF', icao: 'BIKF', name: 'Keflavík Intl',              city: 'Reykjavík',country: 'IS', lat: 63.9850,  lon: -22.6056,  size: 'large' },
  { iata: 'OSL', icao: 'ENGM', name: 'Oslo Gardermoen',                 city: 'Oslo',          country: 'NO', lat: 60.1939,  lon: 11.1004,   size: 'large' },
  { iata: 'CPH', icao: 'EKCH', name: 'Copenhagen Kastrup',              city: 'Copenhagen',    country: 'DK', lat: 55.6180,  lon: 12.6508,   size: 'large' },
  { iata: 'DUB', icao: 'EIDW', name: 'Dublin',                          city: 'Dublin',        country: 'IE', lat: 53.4213,  lon: -6.2701,   size: 'large' },
  { iata: 'WAW', icao: 'EPWA', name: 'Warsaw Chopin',                   city: 'Warsaw',        country: 'PL', lat: 52.1657,  lon: 20.9671,   size: 'large' },
  { iata: 'PRG', icao: 'LKPR', name: 'Václav Havel',               city: 'Prague',        country: 'CZ', lat: 50.1008,  lon: 14.2600,   size: 'large' },
  { iata: 'VIE', icao: 'LOWW', name: 'Vienna Intl',                     city: 'Vienna',        country: 'AT', lat: 48.1103,  lon: 16.5697,   size: 'large' },
  { iata: 'BRU', icao: 'EBBR', name: 'Brussels',                        city: 'Brussels',      country: 'BE', lat: 50.9014,  lon: 4.4844,    size: 'large' },
  { iata: 'LIS', icao: 'LPPT', name: 'Humberto Delgado',                city: 'Lisbon',        country: 'PT', lat: 38.7813,  lon: -9.1359,   size: 'large' },
  { iata: 'ATH', icao: 'LGAV', name: 'Athens Eleftherios Venizelos',    city: 'Athens',        country: 'GR', lat: 37.9364,  lon: 23.9445,   size: 'large' },
];
