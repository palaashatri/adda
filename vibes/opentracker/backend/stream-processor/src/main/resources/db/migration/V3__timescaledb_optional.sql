-- Optional TimescaleDB hypertable conversion.
--
-- This migration is a no-op on stock PostgreSQL and only takes effect when the
-- TimescaleDB extension is available (e.g. when the postgres image is swapped
-- for `timescale/timescaledb-ha:pg16`). It converts the position tables into
-- hypertables for chunk-based time partitioning, automatic chunk pruning, and
-- per-chunk compression — without any application code changes.
--
-- The DO block guards every statement so the migration succeeds against a
-- vanilla PostgreSQL 16 + PostGIS image as well.

DO $$
BEGIN
    IF EXISTS (SELECT 1 FROM pg_available_extensions WHERE name = 'timescaledb') THEN
        CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

        -- Hypertable conversion is idempotent thanks to if_not_exists => TRUE.
        PERFORM create_hypertable(
            'aircraft_positions',
            'recorded_at',
            chunk_time_interval => INTERVAL '1 day',
            migrate_data        => TRUE,
            if_not_exists       => TRUE
        );

        PERFORM create_hypertable(
            'vessel_positions',
            'recorded_at',
            chunk_time_interval => INTERVAL '1 day',
            migrate_data        => TRUE,
            if_not_exists       => TRUE
        );

        -- Compression policy: compress chunks older than 7 days. Saves >90% on
        -- typical position data while leaving the hot window uncompressed.
        BEGIN
            EXECUTE 'ALTER TABLE aircraft_positions SET (timescaledb.compress, '
                  || 'timescaledb.compress_segmentby = ''aircraft_id'')';
            PERFORM add_compression_policy('aircraft_positions', INTERVAL '7 days', if_not_exists => TRUE);
        EXCEPTION WHEN OTHERS THEN
            -- Older Timescale versions may not expose all options; ignore so the
            -- migration remains idempotent across deployment targets.
            NULL;
        END;

        BEGIN
            EXECUTE 'ALTER TABLE vessel_positions SET (timescaledb.compress, '
                  || 'timescaledb.compress_segmentby = ''vessel_id'')';
            PERFORM add_compression_policy('vessel_positions', INTERVAL '7 days', if_not_exists => TRUE);
        EXCEPTION WHEN OTHERS THEN
            NULL;
        END;
    END IF;
END$$;
