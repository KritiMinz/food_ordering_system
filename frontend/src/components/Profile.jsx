import { useEffect, useState } from "react"
import AddressManager from "./AddressManager"

function Profile({ currentUser, onClose, onSelectAddress }) {
const [profile, setProfile] = useState(null)

const [loading, setLoading] = useState(true)

const [editing, setEditing] = useState(false)

const [saving, setSaving] = useState(false)

const [error, setError] = useState("")

const [success, setSuccess] = useState("")

const [name, setName] = useState("")

const [phone, setPhone] = useState("")

// ==============================
// LOAD PROFILE
// ==============================

useEffect(() => {
const loadProfile = async () => {
try {
setLoading(true)
setError("")

    const token = localStorage.getItem("authToken")

    if (!token) {
      throw new Error(
        "Authentication required. Please login again."
      )
    }

    const response = await fetch(
      "http://localhost:18080/api/profile",
      {
        method: "GET",
        headers: {
          Authorization: `Bearer ${token}`,
        },
      }
    )

    const data = await response.json()

    if (!response.ok) {
      throw new Error(
        data.message || "Failed to load profile"
      )
    }

    setProfile(data.profile)

    setName(data.profile.name || "")

    setPhone(data.profile.phone || "")
  } catch (error) {
    console.error(
      "Failed to load profile:",
      error
    )

    setError(error.message)
  } finally {
    setLoading(false)
  }
}

if (currentUser) {
  loadProfile()
} else {
  setLoading(false)
}

}, [currentUser])

// ==============================
// UPDATE PROFILE
// ==============================

const handleUpdateProfile = async (event) => {
event.preventDefault()

try {
  setSaving(true)
  setError("")
  setSuccess("")

  const token =
    localStorage.getItem("authToken")

  if (!token) {
    throw new Error(
      "Authentication required. Please login again."
    )
  }

  const response = await fetch(
    "http://localhost:18080/api/profile",
    {
      method: "PUT",

      headers: {
        "Content-Type": "application/json",
        Authorization: `Bearer ${token}`,
      },

      body: JSON.stringify({
        name: name.trim(),
        phone: phone.trim(),
      }),
    }
  )

  const data = await response.json()

  if (!response.ok) {
    throw new Error(
      data.message || "Failed to update profile"
    )
  }

  setProfile(data.profile)

  setName(data.profile.name || "")

  setPhone(data.profile.phone || "")

  setEditing(false)

  setSuccess(
    data.message ||
    "Profile updated successfully"
  )

  // Keep navbar user information updated
  localStorage.setItem(
    "currentUser",
    JSON.stringify(data.profile)
  )

} catch (error) {
  console.error(
    "Failed to update profile:",
    error
  )

  setError(error.message)
} finally {
  setSaving(false)
}

}

// ==============================
// NOT LOGGED IN
// ==============================

if (!currentUser) {
return ( <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

    <div className="bg-white rounded-2xl shadow-xl w-full max-w-md p-8 text-center">

      <div className="text-5xl mb-4">
        🔐
      </div>

      <h2 className="text-2xl font-bold text-gray-800">
        Login to view your profile
      </h2>

      <button
        onClick={onClose}
        className="mt-6 bg-orange-600 text-white px-6 py-3 rounded-lg
                   font-semibold hover:bg-orange-700"
      >
        Close
      </button>

    </div>

  </div>
)

}

// ==============================
// LOADING
// ==============================

if (loading) {
return ( <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

    <div className="bg-white rounded-2xl shadow-xl w-full max-w-md p-10 text-center">

      <p className="text-gray-600">
        Loading your profile...
      </p>

    </div>

  </div>
)

}

// ==============================
// ERROR
// ==============================

if (error && !profile) {
return ( <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

    <div className="bg-white rounded-2xl shadow-xl w-full max-w-md p-8">

      <div className="bg-red-100 text-red-700 rounded-lg p-4">
        {error}
      </div>

      <button
        onClick={onClose}
        className="w-full mt-6 bg-gray-800 text-white py-3 rounded-lg
                   font-semibold hover:bg-gray-900"
      >
        Close
      </button>

    </div>

  </div>
)


}

// ==============================
// PROFILE
// ==============================

return ( <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">


  <div className="bg-white rounded-2xl shadow-xl w-full max-w-2xl max-h-[90vh] overflow-y-auto">

    {/* ==============================
        HEADER
    ============================== */}

    <div className="bg-orange-600 text-white p-8">

      <div className="flex items-center justify-between">

        <div className="flex items-center gap-5">

          <div className="w-20 h-20 rounded-full bg-white
                          text-orange-600 flex items-center
                          justify-center text-3xl font-bold">

            {profile?.name?.charAt(0).toUpperCase()}

          </div>

          <div>

            <h2 className="text-3xl font-bold">
              My Profile
            </h2>

            <p className="text-orange-100 mt-1">
              Manage your account information
            </p>

          </div>

        </div>

        <button
          onClick={onClose}
          className="text-white text-2xl hover:text-orange-200"
        >
          ✕
        </button>

      </div>

    </div>


    {/* ==============================
        CONTENT
    ============================== */}

    <div className="p-8">

      {success && (
        <div className="bg-green-100 text-green-700 rounded-lg p-4 mb-6">
          {success}
        </div>
      )}

      {error && (
        <div className="bg-red-100 text-red-700 rounded-lg p-4 mb-6">
          {error}
        </div>
      )}


      {!editing ? (

        <>
          <h3 className="text-xl font-bold text-gray-800 mb-6">
            Personal Information
          </h3>

          <div className="space-y-5">

            <div>
              <p className="text-sm text-gray-500">
                Full Name
              </p>

              <p className="text-lg font-semibold text-gray-800">
                {profile?.name}
              </p>
            </div>


            <div>
              <p className="text-sm text-gray-500">
                Email Address
              </p>

              <p className="text-lg font-semibold text-gray-800">
                {profile?.email}
              </p>
            </div>


            <div>
              <p className="text-sm text-gray-500">
                Phone Number
              </p>

              <p className="text-lg font-semibold text-gray-800">
                {profile?.phone || "Not provided"}
              </p>
            </div>


            <div>
              <p className="text-sm text-gray-500">
                Account Type
              </p>

              <span className="inline-block mt-1
                               bg-orange-100 text-orange-700
                               px-3 py-1 rounded-full
                               text-sm font-semibold">

                {profile?.role}

              </span>
            </div>

          </div>


          <button
            onClick={() => {
              setEditing(true)
              setSuccess("")
              setError("")
            }}
            className="w-full mt-8 bg-orange-600 text-white py-3 rounded-lg
                       font-semibold hover:bg-orange-700"
          >
            ✏️ Edit Profile
          </button>

          <div className="border-t mt-10 pt-8">

  <AddressManager
  onSelectAddress={(address) => {
    onSelectAddress(address)
  }}
/>

</div>

        </>

      ) : (

        <form
          onSubmit={handleUpdateProfile}
        >

          <h3 className="text-xl font-bold text-gray-800 mb-6">
            Edit Profile
          </h3>


          {/* NAME */}

          <div className="mb-5">

            <label className="block text-sm font-semibold text-gray-700 mb-2">
              Full Name
            </label>

            <input
              type="text"
              value={name}
              onChange={(event) =>
                setName(event.target.value)
              }
              required
              className="w-full px-4 py-3 rounded-lg border border-gray-300
                         focus:outline-none focus:ring-2 focus:ring-orange-500"
            />

          </div>


          {/* EMAIL */}

          <div className="mb-5">

            <label className="block text-sm font-semibold text-gray-700 mb-2">
              Email Address
            </label>

            <input
              type="email"
              value={profile?.email || ""}
              disabled
              className="w-full px-4 py-3 rounded-lg border border-gray-200
                         bg-gray-100 text-gray-500 cursor-not-allowed"
            />

            <p className="text-xs text-gray-500 mt-1">
              Email address cannot be changed.
            </p>

          </div>


          {/* PHONE */}

          <div className="mb-6">

            <label className="block text-sm font-semibold text-gray-700 mb-2">
              Phone Number
            </label>

            <input
              type="tel"
              value={phone}
              onChange={(event) =>
                setPhone(event.target.value)
              }
              required
              className="w-full px-4 py-3 rounded-lg border border-gray-300
                         focus:outline-none focus:ring-2 focus:ring-orange-500"
            />

          </div>


          {/* BUTTONS */}

          <div className="flex gap-4">

            <button
              type="button"
              onClick={() => {
                setEditing(false)
                setError("")
                setSuccess("")
                setName(profile?.name || "")
                setPhone(profile?.phone || "")
              }}
              disabled={saving}
              className="flex-1 border border-gray-300 text-gray-700 py-3 rounded-lg
                         font-semibold hover:bg-gray-50"
            >
              Cancel
            </button>


            <button
              type="submit"
              disabled={saving}
              className="flex-1 bg-orange-600 text-white py-3 rounded-lg
                         font-semibold hover:bg-orange-700
                         disabled:opacity-50"
            >
              {saving ? "Saving..." : "Save Changes"}
            </button>

          </div>

        </form>

      )}

    </div>

  </div>

</div>

)
}

export default Profile
