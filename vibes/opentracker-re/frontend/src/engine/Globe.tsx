import React from 'react';
import DeckGL from '@deck.gl/react';
import { SimpleMeshLayer } from '@deck.gl/mesh-layers';
import { ConeGeometry } from '@luma.gl/engine';
import type { Entity } from '../network/useTelemetry';

interface GlobeProps {
    entities: Entity[];
    onViewportChange?: (viewState: any) => void;
    onSelectEntity?: (entity: Entity) => void;
}

const INITIAL_VIEW_STATE = {
    longitude: -74,
    latitude: 40.7,
    zoom: 3,
    pitch: 45,
    bearing: 0
};

export const Globe: React.FC<GlobeProps> = ({ entities, onViewportChange, onSelectEntity }) => {
    const layers = [
        new SimpleMeshLayer({
            id: 'entity-mesh-layer',
            data: entities,
            pickable: true,
            mesh: new ConeGeometry({
                radius: 1,
                height: 3,
                cap: true
            }),
            getPosition: (d: Entity) => [d.longitude, d.latitude, d.altitude],
            getColor: (d: Entity) => {
                if (d.type === 'FLIGHT') return [0, 150, 255];
                if (d.type === 'SHIP') return [0, 255, 150];
                return [255, 150, 0];
            },
            getOrientation: (d: Entity) => [0, -d.heading, 0],
            getScale: (d: Entity) => [2000, 2000, 2000],
            onClick: (info) => {
                if (info.object && onSelectEntity) {
                    onSelectEntity(info.object);
                }
            }
        })
    ];

    return (
        <DeckGL
            initialViewState={INITIAL_VIEW_STATE}
            controller={true}
            layers={layers}
            onViewStateChange={({ viewState }) => {
                if (onViewportChange) onViewportChange(viewState);
            }}
        />
    );
};
