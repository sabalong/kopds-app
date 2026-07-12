BEGIN;

CREATE SEQUENCE member_number_seq START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE FUNCTION next_member_number()
RETURNS TEXT
LANGUAGE plpgsql
AS $$
DECLARE
  sequence_value BIGINT := nextval('member_number_seq');
  sequence_text TEXT;
BEGIN
  sequence_text := sequence_value::TEXT;
  RETURN 'KOPDS-' || lpad(
    sequence_text,
    greatest(4, length(sequence_text)),
    '0'
  );
END;
$$;

CREATE OR REPLACE FUNCTION set_updated_at()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
BEGIN
  NEW.updated_at := CURRENT_TIMESTAMP;
  RETURN NEW;
END;
$$;

CREATE TABLE member (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  member_number TEXT NOT NULL DEFAULT next_member_number(),
  full_name TEXT NOT NULL CHECK (btrim(full_name) <> ''),
  created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
  deleted_at TIMESTAMPTZ NULL,
  CONSTRAINT member_number_unique UNIQUE (member_number)
);

CREATE TABLE contribution_type (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  code TEXT NOT NULL CHECK (code ~ '^[A-Z0-9_-]+$'),
  category TEXT NOT NULL CHECK (btrim(category) <> ''),
  is_mandatory BOOLEAN NOT NULL DEFAULT FALSE,
  requires_active_loan BOOLEAN NOT NULL DEFAULT FALSE,
  is_refundable BOOLEAN NOT NULL DEFAULT FALSE,
  display_order INTEGER NOT NULL DEFAULT 0 CHECK (display_order >= 0),
  active BOOLEAN NOT NULL DEFAULT TRUE,
  created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
  deleted_at TIMESTAMPTZ NULL
);

CREATE UNIQUE INDEX contribution_type_code_active_unique
  ON contribution_type (code)
  WHERE deleted_at IS NULL;

CREATE TABLE contribution_type_i18n (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  contribution_type_id UUID NOT NULL REFERENCES contribution_type(id) ON DELETE RESTRICT,
  locale TEXT NOT NULL CHECK (btrim(locale) <> ''),
  name TEXT NOT NULL CHECK (btrim(name) <> ''),
  short_name TEXT NULL,
  description TEXT NULL,
  created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
  deleted_at TIMESTAMPTZ NULL
);

CREATE UNIQUE INDEX contribution_type_i18n_locale_active_unique
  ON contribution_type_i18n (contribution_type_id, locale)
  WHERE deleted_at IS NULL;

CREATE TABLE member_transaction (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  member_id UUID NOT NULL REFERENCES member(id) ON DELETE RESTRICT,
  contribution_type_id UUID NOT NULL REFERENCES contribution_type(id) ON DELETE RESTRICT,
  amount NUMERIC(18, 2) NOT NULL,
  transaction_type TEXT NOT NULL,
  transaction_date TIMESTAMPTZ NOT NULL,
  reference_type TEXT NULL,
  reference_id UUID NULL,
  created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
  deleted_at TIMESTAMPTZ NULL
);

CREATE INDEX member_search_idx ON member (member_number, full_name);
CREATE INDEX contribution_type_display_idx ON contribution_type (display_order, code);
CREATE INDEX member_transaction_member_idx ON member_transaction (member_id, transaction_date);
CREATE INDEX member_transaction_type_idx ON member_transaction (contribution_type_id, transaction_date);

CREATE TRIGGER member_updated_at
BEFORE UPDATE ON member
FOR EACH ROW EXECUTE FUNCTION set_updated_at();

CREATE TRIGGER contribution_type_updated_at
BEFORE UPDATE ON contribution_type
FOR EACH ROW EXECUTE FUNCTION set_updated_at();

CREATE TRIGGER contribution_type_i18n_updated_at
BEFORE UPDATE ON contribution_type_i18n
FOR EACH ROW EXECUTE FUNCTION set_updated_at();

CREATE TRIGGER member_transaction_updated_at
BEFORE UPDATE ON member_transaction
FOR EACH ROW EXECUTE FUNCTION set_updated_at();

COMMIT;
