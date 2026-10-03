import { useEffect, useState } from "react"

function AddressManager({ onSelectAddress }) {
  const [addresses, setAddresses] = useState([])
  const [label, setLabel] = useState("")
  const [address, setAddress] = useState("")
  const [editingId, setEditingId] = useState(null)
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState("")

  const getToken = () => localStorage.getItem("authToken")

  const loadAddresses = async () => {
    try {
      setError("")

      const response = await fetch(
        "http://localhost:18080/api/addresses",
        {
          headers: {
            Authorization: `Bearer ${getToken()}`,
          },
        }
      )

      const data = await response.json()

      if (!response.ok) {
        throw new Error(
          data.message || "Unable to load addresses"
        )
      }

      setAddresses(data.addresses || [])
    } catch (error) {
      setError(error.message)
    }
  }

  useEffect(() => {
    loadAddresses()
  }, [])

  const resetForm = () => {
    setLabel("")
    setAddress("")
    setEditingId(null)
  }

  const handleSubmit = async (event) => {
    event.preventDefault()

    if (!label.trim() || !address.trim()) {
      setError("Please enter both label and address.")
      return
    }

    setLoading(true)
    setError("")

    try {
      const isEditing = editingId !== null

      const response = await fetch(
        isEditing
          ? `http://localhost:18080/api/addresses/${editingId}`
          : "http://localhost:18080/api/addresses",
        {
          method: isEditing ? "PUT" : "POST",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${getToken()}`,
          },
          body: JSON.stringify({
            label: label.trim(),
            address: address.trim(),
          }),
        }
      )

      const data = await response.json()

      if (!response.ok) {
        throw new Error(
          data.message || "Unable to save address"
        )
      }

      resetForm()
      await loadAddresses()
    } catch (error) {
      setError(error.message)
    } finally {
      setLoading(false)
    }
  }

  const handleEdit = (savedAddress) => {
    setEditingId(savedAddress.id)
    setLabel(savedAddress.label)
    setAddress(savedAddress.address)
    setError("")
  }

  const handleDelete = async (id) => {
    const confirmed = window.confirm(
      "Are you sure you want to delete this address?"
    )

    if (!confirmed) {
      return
    }

    setError("")

    try {
      const response = await fetch(
        `http://localhost:18080/api/addresses/${id}`,
        {
          method: "DELETE",
          headers: {
            Authorization: `Bearer ${getToken()}`,
          },
        }
      )

      const data = await response.json()

      if (!response.ok) {
        throw new Error(
          data.message || "Unable to delete address"
        )
      }

      await loadAddresses()
    } catch (error) {
      setError(error.message)
    }
  }

  return (
    <div className="space-y-6">

      <div>
        <h3 className="text-xl font-bold text-gray-800 mb-4">
          Saved Addresses
        </h3>

        {addresses.length === 0 ? (
          <p className="text-gray-500">
            No saved addresses yet.
          </p>
        ) : (
          <div className="space-y-3">

            {addresses.map((savedAddress) => (
              <div
                key={savedAddress.id}
                className="border rounded-xl p-4"
              >

                <div className="flex justify-between items-start gap-4">

                  <div>
                    <h4 className="font-bold text-gray-800">
                      {savedAddress.label}
                    </h4>

                    <p className="text-gray-600 mt-1">
                      {savedAddress.address}
                    </p>
                  </div>

                  <div className="flex gap-2">

                    <button
                      type="button"
                      onClick={() =>
                        onSelectAddress(savedAddress.address)
                      }
                      className="bg-orange-600 text-white px-3 py-2 rounded-lg
                                 text-sm font-semibold hover:bg-orange-700"
                    >
                      Use
                    </button>

                    <button
                      type="button"
                      onClick={() => handleEdit(savedAddress)}
                      className="border border-gray-300 px-3 py-2 rounded-lg
                                 text-sm hover:bg-gray-100"
                    >
                      Edit
                    </button>

                    <button
                      type="button"
                      onClick={() =>
                        handleDelete(savedAddress.id)
                      }
                      className="border border-red-300 text-red-600 px-3 py-2
                                 rounded-lg text-sm hover:bg-red-50"
                    >
                      Delete
                    </button>

                  </div>

                </div>

              </div>
            ))}

          </div>
        )}
      </div>

      <div className="border-t pt-5">

        <h3 className="text-xl font-bold text-gray-800 mb-4">
          {editingId !== null
            ? "Edit Address"
            : "Add New Address"}
        </h3>

        <form
          onSubmit={handleSubmit}
          className="space-y-3"
        >

          <input
            type="text"
            value={label}
            onChange={(event) =>
              setLabel(event.target.value)
            }
            placeholder="Label (Home, College, Work)"
            className="w-full border rounded-lg px-4 py-3"
          />

          <textarea
            value={address}
            onChange={(event) =>
              setAddress(event.target.value)
            }
            placeholder="Complete delivery address"
            rows="3"
            className="w-full border rounded-lg px-4 py-3"
          />

          <div className="flex gap-3">

            <button
              type="submit"
              disabled={loading}
              className="bg-orange-600 text-white px-5 py-3 rounded-lg
                         font-semibold hover:bg-orange-700
                         disabled:bg-gray-400"
            >
              {loading
                ? "Saving..."
                : editingId !== null
                  ? "Update Address"
                  : "Save Address"}
            </button>

            {editingId !== null && (
              <button
                type="button"
                onClick={resetForm}
                className="border border-gray-300 px-5 py-3 rounded-lg
                           hover:bg-gray-100"
              >
                Cancel
              </button>
            )}

          </div>

        </form>
      </div>

      {error && (
        <div className="bg-red-100 text-red-700 px-4 py-3 rounded-lg">
          {error}
        </div>
      )}

    </div>
  )
}

export default AddressManager
