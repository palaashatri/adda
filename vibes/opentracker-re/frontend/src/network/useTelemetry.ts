import { useEffect, useState, useRef } from 'react';
import { com } from '../proto/generated';

const GeoEntity = com.aetheris.shared.proto.GeoEntity;

export interface Entity {
    id: string;
    type: string;
    latitude: number;
    longitude: number;
    altitude: number;
    velocity: number;
    heading: number;
    timestamp: number;
    // For interpolation
    prevLat?: number;
    prevLon?: number;
    targetLat?: number;
    targetLon?: number;
    lastUpdate?: number;
}

export function useTelemetry() {
    const [entities, setEntities] = useState<Record<string, Entity>>({});
    const [interpolatedEntities, setInterpolatedEntities] = useState<Entity[]>([]);
    const socketRef = useRef<WebSocket | null>(null);
    const entitiesRef = useRef<Record<string, Entity>>({});

    useEffect(() => {
        const socket = new WebSocket('ws://localhost:8081/ws/telemetry');
        socket.binaryType = 'arraybuffer';
        socketRef.current = socket;

        socket.onmessage = (event) => {
            try {
                const buffer = new Uint8Array(event.data);
                const message = GeoEntity.decode(buffer);
                
                const id = message.id;
                const prev = entitiesRef.current[id];
                
                const newEntity: Entity = {
                    id: message.id,
                    type: GeoEntity.EntityType[message.type],
                    latitude: prev ? prev.latitude : message.latitude,
                    longitude: prev ? prev.longitude : message.longitude,
                    altitude: message.altitude,
                    velocity: message.velocity,
                    heading: message.heading,
                    timestamp: Number(message.timestamp),
                    // Interpolation state
                    prevLat: prev ? prev.targetLat : message.latitude,
                    prevLon: prev ? prev.targetLon : message.longitude,
                    targetLat: message.latitude,
                    targetLon: message.longitude,
                    lastUpdate: Date.now()
                };
                
                entitiesRef.current[id] = newEntity;
                setEntities({ ...entitiesRef.current });
            } catch (err) {
                console.error("Protobuf decode error", err);
            }
        };

        return () => socket.close();
    }, []);

    // Animation Loop for interpolation
    useEffect(() => {
        let frameId: number;
        
        const animate = () => {
            const now = Date.now();
            const interpolated = Object.values(entitiesRef.current).map(e => {
                if (!e.lastUpdate || !e.prevLat || !e.targetLat) return e;
                
                // Interpolate over 1 second (1000ms) - matching typical update frequency
                const duration = 1000; 
                const elapsed = now - e.lastUpdate;
                const t = Math.min(elapsed / duration, 1.0);
                
                // Simple linear interpolation (Hermite could be added for better curves)
                return {
                    ...e,
                    latitude: e.prevLat + (e.targetLat - e.prevLat) * t,
                    longitude: e.prevLon + (e.targetLon - e.prevLon) * t
                };
            });
            
            setInterpolatedEntities(interpolated);
            frameId = requestAnimationFrame(animate);
        };
        
        frameId = requestAnimationFrame(animate);
        return () => cancelAnimationFrame(frameId);
    }, []);

    const updateViewport = (bounds: { minLat: number; maxLat: number; minLon: number; maxLon: number }) => {
        if (socketRef.current?.readyState === WebSocket.OPEN) {
            socketRef.current.send(JSON.stringify(bounds));
        }
    };

    return { entities: interpolatedEntities, updateViewport };
}
