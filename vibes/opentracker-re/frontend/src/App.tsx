import { useState, useEffect, useCallback } from 'react';
import { Globe } from './engine/Globe';
import { useTelemetry, Entity } from './network/useTelemetry';
import { Search, Star, Settings, Info, Plane, Ship, Satellite } from 'lucide-react';

function App() {
  const { entities, updateViewport } = useTelemetry();
  const [selectedEntity, setSelectedEntity] = useState<Entity | null>(null);
  const [metadata, setMetadata] = useState<any>(null);
  const [showSearch, setShowSearch] = useState(false);
  const [searchQuery, setSearchQuery] = useState('');

  useEffect(() => {
    if (selectedEntity) {
      fetch('http://localhost:8080/graphql', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          query: `
            query GetMetadata($id: ID!) {
              entityMetadata(id: $id) {
                name
                country
                flight { airline origin destination aircraftType }
                ship { vesselName vesselType callsign }
              }
            }
          `,
          variables: { id: selectedEntity.id }
        })
      })
      .then(res => res.json())
      .then(data => setMetadata(data.data?.entityMetadata))
      .catch(err => console.error("GraphQL Error:", err));
    } else {
      setMetadata(null);
    }
  }, [selectedEntity]);

  const handleViewportChange = (viewState: any) => {
    updateViewport({
      minLat: viewState.latitude - 10,
      maxLat: viewState.latitude + 10,
      minLon: viewState.longitude - 20,
      maxLon: viewState.longitude + 20
    });
  };

  const handleKeyDown = useCallback((e: KeyboardEvent) => {
    if ((e.metaKey || e.ctrlKey) && e.key === 'k') {
      e.preventDefault();
      setShowSearch(prev => !prev);
    }
    if (e.key === 'Escape') {
      setShowSearch(false);
    }
  }, []);

  useEffect(() => {
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [handleKeyDown]);

  const filteredEntities = searchQuery 
    ? entities.filter(e => e.id.toLowerCase().includes(searchQuery.toLowerCase()))
    : [];

  return (
    <div className="h-screen w-screen overflow-hidden bg-black text-white font-sans">
      {/* 3D Engine */}
      <Globe 
        entities={entities} 
        onViewportChange={handleViewportChange} 
        onSelectEntity={setSelectedEntity}
      />

      {/* macOS Sidebar */}
      <div className="mac-sidebar mac-vibrancy">
        <h1 className="text-xl font-bold tracking-tight mb-8">Aetheris</h1>
        
        <div className="flex flex-col gap-1">
          <div className="flex items-center gap-3 p-2 rounded-lg bg-white/10">
            <Star size={18} /> <span className="text-sm font-medium">Favorites</span>
          </div>
          <div className="flex items-center gap-3 p-2 rounded-lg text-white/60 hover:bg-white/5 cursor-pointer">
            <Plane size={18} /> <span className="text-sm font-medium">Aviation</span>
          </div>
          <div className="flex items-center gap-3 p-2 rounded-lg text-white/60 hover:bg-white/5 cursor-pointer">
            <Ship size={18} /> <span className="text-sm font-medium">Maritime</span>
          </div>
          <div className="flex items-center gap-3 p-2 rounded-lg text-white/60 hover:bg-white/5 cursor-pointer">
            <Satellite size={18} /> <span className="text-sm font-medium">Orbital</span>
          </div>
        </div>

        <div className="mt-auto flex flex-col gap-1 border-t border-white/10 pt-4">
          <div className="flex items-center gap-3 p-2 text-white/40 hover:text-white/60 cursor-pointer"><Settings size={18} /> <span className="text-sm">Settings</span></div>
          <div className="flex items-center gap-3 p-2 text-white/40 hover:text-white/60 cursor-pointer"><Info size={18} /> <span className="text-sm">About</span></div>
        </div>
      </div>

      {/* Spotlight Search */}
      {showSearch && (
        <div className="fixed inset-0 z-50 flex items-start justify-center pt-[20vh] bg-black/20 backdrop-blur-sm" onClick={() => setShowSearch(false)}>
          <div className="spotlight-search mac-vibrancy w-[600px] overflow-hidden flex flex-col" onClick={e => e.stopPropagation()}>
            <div className="flex items-center p-4 border-b border-white/10">
              <Search size={22} className="text-white/40 mr-3" />
              <input 
                autoFocus
                type="text" 
                placeholder="Search flight, vessel, or satellite..." 
                className="bg-transparent border-none outline-none w-full text-xl text-white placeholder:text-white/20"
                value={searchQuery}
                onChange={e => setSearchQuery(e.target.value)}
              />
            </div>
            {filteredEntities.length > 0 && (
              <div className="max-h-[300px] overflow-y-auto p-2">
                {filteredEntities.map(entity => (
                  <div 
                    key={entity.id} 
                    className="p-3 rounded-lg hover:bg-white/10 cursor-pointer flex justify-between items-center"
                    onClick={() => {
                      setSelectedEntity(entity);
                      setShowSearch(false);
                      setSearchQuery('');
                    }}
                  >
                    <div className="flex items-center gap-3">
                      {entity.type === 'FLIGHT' && <Plane size={18} className="text-blue-400" />}
                      {entity.type === 'SHIP' && <Ship size={18} className="text-green-400" />}
                      {entity.type === 'SATELLITE' && <Satellite size={18} className="text-orange-400" />}
                      <span className="font-semibold">{entity.id}</span>
                    </div>
                    <div className="text-xs text-white/40 uppercase tracking-widest">{entity.type}</div>
                  </div>
                ))}
              </div>
            )}
          </div>
        </div>
      )}

      {/* Inspector Panel */}
      {selectedEntity && (
        <div className="inspector-panel mac-panel mac-vibrancy">
          <div className="flex justify-between items-start mb-6">
            <div>
              <h2 className="text-2xl font-bold tracking-tight">{metadata?.name || selectedEntity.id}</h2>
              <p className="text-xs text-blue-400 font-mono tracking-tighter uppercase">{selectedEntity.type} • {metadata?.country}</p>
            </div>
            <button 
              onClick={() => setSelectedEntity(null)}
              className="text-white/40 hover:text-white"
            >✕</button>
          </div>
          
          {selectedEntity.type === 'FLIGHT' && metadata?.flight && (
            <div className="mb-6 p-4 bg-white/5 rounded-2xl border border-white/10">
                <div className="flex justify-between items-center mb-4">
                    <div className="text-center">
                        <p className="text-[10px] text-white/40 uppercase font-black">Origin</p>
                        <p className="text-2xl font-bold">{metadata.flight.origin}</p>
                    </div>
                    <div className="flex-1 flex flex-col items-center px-4">
                        <div className="w-full h-[1px] bg-white/20 relative">
                            <Plane size={14} className="absolute left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2 text-blue-400" />
                        </div>
                    </div>
                    <div className="text-center">
                        <p className="text-[10px] text-white/40 uppercase font-black">Dest</p>
                        <p className="text-2xl font-bold">{metadata.flight.destination}</p>
                    </div>
                </div>
                <p className="text-xs text-center text-white/60 font-medium">{metadata.flight.airline}</p>
            </div>
          )}

          <div className="grid grid-cols-2 gap-4">
            <div className="bg-white/5 p-3 rounded-xl">
              <p className="text-[10px] text-white/40 uppercase font-bold tracking-widest mb-1">Altitude</p>
              <p className="text-lg font-mono">{selectedEntity.altitude.toFixed(0)}m</p>
            </div>
            <div className="bg-white/5 p-3 rounded-xl">
              <p className="text-[10px] text-white/40 uppercase font-bold tracking-widest mb-1">Velocity</p>
              <p className="text-lg font-mono">{selectedEntity.velocity.toFixed(0)}kt</p>
            </div>
            <div className="bg-white/5 p-3 rounded-xl">
              <p className="text-[10px] text-white/40 uppercase font-bold tracking-widest mb-1">Heading</p>
              <p className="text-lg font-mono">{selectedEntity.heading.toFixed(0)}°</p>
            </div>
            <div className="bg-white/5 p-3 rounded-xl">
              <p className="text-[10px] text-white/40 uppercase font-bold tracking-widest mb-1">Position</p>
              <p className="text-[11px] font-mono">{selectedEntity.latitude.toFixed(4)}, {selectedEntity.longitude.toFixed(4)}</p>
            </div>
          </div>

          <div className="mt-8 border-t border-white/10 pt-6">
             <div className="flex items-center justify-between mb-4">
               <span className="text-sm font-medium">History Trace</span>
               <div className="h-2 w-12 bg-blue-500 rounded-full blur-[2px]"></div>
             </div>
             <p className="text-xs text-white/40 italic">Live telemetry active...</p>
          </div>
        </div>
      )}
    </div>
  );
}

export default App;
